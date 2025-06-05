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

#define MPIX_QOI_PUT_BYTE(buf, sz, i, c) if ((i) >= (qoi_sz)) return (i); qoi_buf[(i)++] = (c);

static inline size_t mpix_qoi_encode()
{
	MPIX_INF("");

	return 1;
}

void mpix_qoi_encode_rgb24_op(struct mpix_base_op *base)
{
	const uint8_t *buf_in = mpix_op_get_input_line(base);
	size_t sz_out = 0;
	uint8_t *buf_out = mpix_op_peek_output(base, &sz_out);
	size_t o = 0;

	for (size_t w = 0; w + 2 <= base->width; w += 2) {
		uint8_t idx0 = (buf_in[w / 2] & 0xf0) >> 4;
		uint8_t idx1 = (buf_in[w / 2] & 0x0f) >> 0;

		o += mpix_qoi_encode(idx0, buf_out + o, sz_out - o);
		o += mpix_qoi_encode(idx1, buf_out + o, sz_out - o);
	}

	mpix_op_done(base);
}
MPIX_REGISTER_QOI_CONVERT_OP(encode_rgb24, mpix_qoi_encode_rgb24_op, RGB24, QOI);

static inline size_t mpix_qoi_depalettize()
{
	MPIX_INF("");

	return 1;
}

void mpix_qoi_depalettize_op(struct mpix_base_op *base)
{
	const uint8_t *buf_in = mpix_op_get_input_line(base);
	size_t sz_out = 0;
	uint8_t *buf_out = mpix_op_peek_output(base, &sz_out);
	size_t o = 0;

	for (size_t w = 0; w + 2 <= base->width; w += 2) {
		uint8_t idx0 = (buf_in[w / 2] & 0xf0) >> 4;
		uint8_t idx1 = (buf_in[w / 2] & 0x0f) >> 0;

		o += mpix_qoi_decode(idx0, buf_out + o, sz_out - o);
		o += mpix_qoi_decode(idx1, buf_out + o, sz_out - o);
	}

	mpix_op_done(base);
}
MPIX_REGISTER_QOI_PALETTE_OP(encode_palette1, mpix_qoi_depalettize_op, PALETTE1, QOI);

static const struct mpix_qoi_palette_op **mpix_qoi_palette_op_list =
	(const struct mpix_qoi_palette_op *[]){MPIX_LIST_QOI_PALETTE_OP};

static const struct mpix_qoi_convert_op **mpix_qoi_convert_op_list =
	(const struct mpix_qoi_convert_op *[]){MPIX_LIST_QOI_CONVERT_OP};
