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

#define MPIX_QOI_OP_INDEX  0x00 /* 00xxxxxx */
#define MPIX_QOI_OP_DIFF   0x40 /* 01xxxxxx */
#define MPIX_QOI_OP_LUMA   0x80 /* 10xxxxxx */
#define MPIX_QOI_OP_RUN    0xc0 /* 11xxxxxx */
#define MPIX_QOI_OP_RGB    0xfe /* 11111110 */
#define MPIX_QOI_OP_RGBA   0xff /* 11111111 */

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
	if (o >= dst_sz) {                                                                         \
		return o;                                                                          \
	}                                                                                          \
	dst[o++] = (u);                                                                            \
})

#define MPIX_QOI_PUT_U32(u) ({                                                                     \
	if (o >= dst_sz) {                                                                         \
		return o;                                                                          \
	}                                                                                          \
	dst[o++] = (u) >> 24;                                                                      \
	dst[o++] = (u) >> 16;                                                                      \
	dst[o++] = (u) >> 8;                                                                       \
	dst[o++] = (u) >> 0;                                                                       \
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

#define printf(...) ((void)0)

static inline size_t mpix_qoi_encode_rgb24(struct mpix_qoi_convert_op *op, const uint8_t *src,
					   uint8_t *dst, size_t dst_sz, bool is_last)
{
	const uint8_t r = src[0];
	const uint8_t g = src[1];
	const uint8_t b = src[2];
	const uint8_t a = 0xff;
	uint8_t cache_idx;
	size_t o = 0;

	printf("pixel #%02x%02x%02x, prev #%02x%02x%02x",
		r, g, b, op->qoi_prev[0], op->qoi_prev[1], op->qoi_prev[2]);

	/* Handle run-length operation */
	if (memcmp(op->qoi_prev, src, 3) == 0) {
		printf(" same as prev");

		/* Increase run-length if same pixel */
		op->qoi_run_length++;

		/* Flush current run-length if reaching the maximum */
		if (op->qoi_run_length >= 62 || is_last) {
			printf(" flushing long run");
			MPIX_QOI_PUT_U8(MPIX_QOI_OP_RUN | (op->qoi_run_length - 1));
			op->qoi_run_length = 0;
		}
		printf("\n");
		return o;
	} else {
		/* Flush current run-length if different pixel */
		if (op->qoi_run_length > 0) {
			printf(" flushing run");

			MPIX_QOI_PUT_U8(MPIX_QOI_OP_RUN | (op->qoi_run_length - 1));
			op->qoi_run_length = 0;
		}
	}

	/* Handle run-length operation */
	cache_idx = ((uint16_t)r * 3 + (uint16_t)g * 5 + (uint16_t)b * 7 + (uint16_t)a * 11) % 64;
	if (memcmp(&op->qoi_cache[3 * cache_idx], src, 3) == 0) {
		printf(" OP_INDEX: idx %u\n", cache_idx);
		/* Use table encoding if previously encountered */
		MPIX_QOI_PUT_U8(MPIX_QOI_OP_INDEX | cache_idx);
		memcpy(op->qoi_prev, src, 3);
		return o;
	} else {
		/* Cache the pixel in the array since it is different */
		memcpy(&op->qoi_cache[3 * cache_idx], src, 3);
	}

	/* Relative difference with the previous pixels with wrap arround */
	int8_t dr = r - op->qoi_prev[0];
	int8_t dg = g - op->qoi_prev[1];
	int8_t db = b - op->qoi_prev[2];

	/* Difference between the green channel and the red/blue channel */
	int8_t dgr = dr - dg;
	int8_t dgb = db - dg;

	/* Handle difference operation */
	if (IN_RANGE(dr, -2, 1) && IN_RANGE(dg, -2, 1) && IN_RANGE(db, -2, 1)) {
		printf(" OP_DIFF: dr %d, dg %d, db %d\n", dr, dg, db);
		MPIX_QOI_PUT_U8(MPIX_QOI_OP_DIFF | (dr + 2) << 4 | (dg + 2) << 2 | (db + 2) << 0);
		memcpy(op->qoi_prev, src, 3);
		return o;
	}

	/* Handle luma operation */
	if (IN_RANGE(dgr, -8, 7) && IN_RANGE(dg, -32, 31) && IN_RANGE(dgb, -8, 7)) {
		printf(" OP_LUMA: dgr %d, dg %d, dgb %d\n", dgr, dg, dgb);
		MPIX_QOI_PUT_U8(MPIX_QOI_OP_LUMA | (dg + 32));
		MPIX_QOI_PUT_U8((dgr + 8) << 4 | (dgb + 8) << 0);
		memcpy(op->qoi_prev, src, 3);
		return o;
	}

	/* Handle full RGB operation as fallback */
	printf(" OP_RGB\n");
	MPIX_QOI_PUT_U8(MPIX_QOI_OP_RGB);
	MPIX_QOI_PUT_U8(r);
	MPIX_QOI_PUT_U8(g);
	MPIX_QOI_PUT_U8(b);

	memcpy(op->qoi_prev, src, 3);
	return o;
}

void mpix_qoi_encode_rgb24_op(struct mpix_base_op *base)
{
	struct mpix_qoi_convert_op *op = (void *)base;
	bool first = (base->line_offset == 0);
	const uint8_t *src = mpix_op_get_input_line(base);
	size_t dst_sz = 0;
	uint8_t *dst = mpix_op_peek_output(base, &dst_sz);
	bool is_last = (base->line_offset == base->height);
	size_t o = 0;

	assert(base->width > 0);

	if (first) {
		o += mpix_qoi_add_header(op, dst + o, dst_sz - o);
	}

	for (size_t w = 0; w < base->width - 1; w++, src += 3) {
		o += mpix_qoi_encode_rgb24(op, src, dst + o, dst_sz - o, false);
	}
	o += mpix_qoi_encode_rgb24(op, src, dst + o, dst_sz - o, is_last);

	if (is_last && o + 8 < dst_sz) {
		dst[o++] = 0x00, dst[o++] = 0x00, dst[o++] = 0x00, dst[o++] = 0x00;
		dst[o++] = 0x00, dst[o++] = 0x00, dst[o++] = 0x00, dst[o++] = 0x01;
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
