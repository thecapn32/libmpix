/* SPDX-License-Identifier: Apache-2.0 */

#include <assert.h>
#include <errno.h>

#include <mpix/formats.h>
#include <mpix/genlist.h>
#include <mpix/image.h>
#include <mpix/op_palettize.h>
#include <mpix/op_qoi.h>
#include <mpix/utils.h>

static const struct mpix_qoi_convert_op **mpix_qoi_convert_op_list;
static const struct mpix_qoi_palette_op **mpix_qoi_palette_op_list;

#define MPIX_QOI_OP_RGB   0xfe
#define MPIX_QOI_OP_RGBA  0xff
#define MPIX_QOI_OP_INDEX 0x00
#define MPIX_QOI_OP_DIFF  0x40
#define MPIX_QOI_OP_LUMA  0x80
#define MPIX_QOI_OP_RUN   0xc0

int mpix_image_qoi_depalettize(struct mpix_image *img, size_t max_sz, struct mpix_palette *plt)
{
	const struct mpix_qoi_palette_op *op = NULL;
	struct mpix_qoi_palette_op *new;
	uint8_t bpp = mpix_bits_per_pixel(img->format);
	size_t pitch = img->width * bpp / BITS_PER_BYTE;
	int ret;

	/* TODO: check that the image format is PALETTE# */

	op = mpix_op_by_format(mpix_qoi_palette_op_list, img->format, MPIX_FMT_QOI);
	if (op == NULL) {
		MPIX_ERR("Conversion operation from %s to %s not found",
			 MPIX_FOURCC_TO_STR(img->format), MPIX_FOURCC_TO_STR(MPIX_FMT_QOI));
		return mpix_image_error(img, -ENOSYS);
	}

	ret = mpix_image_append_op(img, &op->base, sizeof(*op), max_sz, pitch);
	if (ret != 0) {
		return ret;
	}

	new = (struct mpix_qoi_palette_op *)img->ops.last;
	new->palette = plt;

	return 0;
}

int mpix_image_qoi_encode(struct mpix_image *img, size_t max_sz)
{
	struct mpix_qoi_convert_op *op = NULL;

	op = mpix_op_by_format(mpix_qoi_convert_op_list, img->format, MPIX_FMT_QOI);
	if (op == NULL) {
		MPIX_ERR("Conversion operation from %s to %s not found",
			 MPIX_FOURCC_TO_STR(img->format), MPIX_FOURCC_TO_STR(MPIX_FMT_QOI));
		return mpix_image_error(img, -ENOSYS);
	}

	return mpix_image_append_op(img, &op->base, sizeof(*op), max_sz, mpix_op_pitch(&op->base));
}

#define MPIX_QOI_PUT_U8(u) ({                                                                      \
	if ((o) >= (dst_sz)) {                                                                         \
		return (o);                                                                        \
	}                                                                                          \
	(dst)[(o)++] = (u);                                                                        \
})

#define MPIX_QOI_PUT_U32(u) ({                                                                     \
	if ((o) >= (dst_sz)) {                                                                         \
		return (o);                                                                        \
	}                                                                                          \
	(dst)[o] = mpix_htobe32(u);                                                                \
	(o) += 4;                                                                                  \
})

static inline size_t mpix_qoi_add_header(struct mpix_qoi_convert_op *op,
					 uint8_t *dst, size_t dst_sz)
{
	size_t o = 0;

	/* magic */
	MPIX_QOI_PUT_U8('q');
	MPIX_QOI_PUT_U8('o');
	MPIX_QOI_PUT_U8('i');
	MPIX_QOI_PUT_U8('f');

	/* width */
	MPIX_QOI_PUT_U32(op->base.width);

	/* height */
	MPIX_QOI_PUT_U32(op->base.height);

	/* channels */
	MPIX_QOI_PUT_U8(3);

	/* colorspace */
	MPIX_QOI_PUT_U8(0);

	return o;
}

static inline size_t mpix_qoi_encode_rgb24(struct mpix_qoi_convert_op *op, const uint8_t *src,
					   uint8_t *dst, size_t dst_sz)
{
	const uint8_t r = src[0];
	const uint8_t g = src[1];
	const uint8_t b = src[2];
	const uint8_t a = 0;
	uint8_t cache_idx;
	size_t o = 0;

	/* Handle run-length operation */
	if (memcmp(op->qoi_prev, src, 3) == 0) {
		/* Increase run-length if same pixel */
		op->qoi_run_length++;

		/* Flush current run-length if reaching the maximum */
		if (op->qoi_run_length >= 62) {
			MPIX_QOI_PUT_U8(MPIX_QOI_OP_RUN | (op->qoi_run_length - 1));
			op->qoi_run_length = 0;
		}
		return o;
	} else {
		/* Flush current run-length if different pixel */
		if (op->qoi_run_length > 0) {
			MPIX_QOI_PUT_U8(MPIX_QOI_OP_RUN | (op->qoi_run_length - 1));
			op->qoi_run_length = 0;
		}
	}

	/* Handle run-length operation */
	cache_idx = (r * 3 + g * 5 + b * 7 + a * 11) % 64;
	if (memcmp(&op->qoi_cache[3 * cache_idx], src, 3) == 0) {
		/* Use table encoding if previously encountered */
		MPIX_QOI_PUT_U8(MPIX_QOI_OP_INDEX | cache_idx);
		return o;
	}
	/* Cache the pixel in the array since it is different */
	memcpy(&op->qoi_cache[3 * cache_idx], src, 3);

	/* Relative difference with the previous pixels with wrap arround */
	int8_t dr = op->qoi_prev[0] - r;
	int8_t dg = op->qoi_prev[1] - g;
	int8_t db = op->qoi_prev[2] - b;

	/* Difference between the green channel and the red/blue channel */
	int8_t dgr = dr - dg;
	int8_t dgb = db - dg;

	/* Handle difference operation */
	if (IN_RANGE(dr, -2, 1) && IN_RANGE(dg, -2, 1) && IN_RANGE(db, -2, 1)) {
		MPIX_QOI_PUT_U8(MPIX_QOI_OP_DIFF | (dr + 2) << 4 | (dg + 2) << 2 | (db + 2) << 0);
		return o;
	}

	/* Handle luma operation */
	if (IN_RANGE(dgr, -8, 7) && IN_RANGE(dg, -32, 31) && IN_RANGE(dgb, -8, 7)) {
		MPIX_QOI_PUT_U8(MPIX_QOI_OP_LUMA | (dg + 32));
		MPIX_QOI_PUT_U8((dgr + 8) << 4 | (dgb + 8) << 0);
	}

	/* Handle full RGB operation as fallback */
	MPIX_QOI_PUT_U8(MPIX_QOI_OP_RGB);
	MPIX_QOI_PUT_U8(r);
	MPIX_QOI_PUT_U8(g);
	MPIX_QOI_PUT_U8(b);

	return o;
}

void mpix_qoi_encode_rgb24_op(struct mpix_base_op *base)
{
	struct mpix_qoi_convert_op *op = (void *)base;
	bool first = (base->line_offset == 0);
	const uint8_t *src = mpix_op_get_input_line(base);
	size_t dst_sz = 0;
	uint8_t *dst = mpix_op_peek_output(base, &dst_sz);
	size_t o = 0;

	if (first) {
		o += mpix_qoi_add_header(op, dst + o, dst_sz - o);
	}

	for (size_t w = 0; w < base->width; w++, src += 3) {
		o += mpix_qoi_encode_rgb24(op, src, dst + o, dst_sz - o);
	}

	mpix_op_get_output_bytes(base, o);
	mpix_op_done(base);
}
MPIX_REGISTER_QOI_CONVERT_OP(encode_rgb24, mpix_qoi_encode_rgb24_op, RGB24, QOI);

static inline size_t mpix_qoi_depalettize(struct mpix_palette_op *op, uint8_t idx,
					  uint8_t *buf, size_t sz)
{
	MPIX_INF("");

	return 1;
}

void mpix_qoi_depalettize_op(struct mpix_base_op *base)
{
	struct mpix_palette_op *op = (void *)base;
	const uint8_t *src = mpix_op_get_input_line(base);
	size_t dst_sz = 0;
	uint8_t *dst = mpix_op_peek_output(base, &dst_sz);
	size_t o = 0;

	for (size_t w = 0; w + 2 <= base->width; w += 2) {
		uint8_t idx0 = (src[w / 2] & 0xf0) >> 4;
		uint8_t idx1 = (src[w / 2] & 0x0f) >> 0;

		o += mpix_qoi_depalettize(op, idx0, dst + o, dst_sz - o);
		o += mpix_qoi_depalettize(op, idx1, dst + o, dst_sz - o);
	}

	mpix_op_done(base);
}
MPIX_REGISTER_QOI_PALETTE_OP(encode_palette1, mpix_qoi_depalettize_op, PALETTE1, QOI);

static const struct mpix_qoi_palette_op **mpix_qoi_palette_op_list =
	(const struct mpix_qoi_palette_op *[]){MPIX_LIST_QOI_PALETTE_OP};

static const struct mpix_qoi_convert_op **mpix_qoi_convert_op_list =
	(const struct mpix_qoi_convert_op *[]){MPIX_LIST_QOI_CONVERT_OP};
