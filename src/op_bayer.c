/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>
#include <errno.h>

#include <mpix/genlist.h>
#include <mpix/image.h>
#include <mpix/op_bayer.h>

static const struct mpix_op **mpix_bayer_op_list;

int mpix_image_debayer(struct mpix_image *img, uint32_t window_size)
{
	const struct mpix_op *op = NULL;

	for (size_t i = 0; mpix_bayer_op_list[i] != NULL; i++) {
		const struct mpix_op *tmp = mpix_bayer_op_list[i];

		if (tmp->format_in == img->format && tmp->format_out == MPIX_FMT_RGB24 &&
		    tmp->window_size == window_size) {
			op = tmp;
			break;
		}
	}

	if (op == NULL) {
		MPIX_ERR("Conversion operation from %s to %s using %ux%u window not found",
			 MPIX_FMT_TO_STR(img->format), MPIX_FOURCC_TO_STR(MPIX_FMT_RGB24),
			 window_size, window_size);
		return mpix_image_error(img, -ENOSYS);
	}

	return mpix_image_append_uncompressed(img, op);
}

#define FOLD_L_3X3(l0, l1, l2)                                                                     \
	{                                                                                          \
		{l0[1], l0[0], l0[1]},                                                             \
		{l1[1], l1[0], l1[1]},                                                             \
		{l2[1], l2[0], l2[1]},                                                             \
	}

#define FOLD_R_3X3(l0, l1, l2, n)                                                                  \
	{                                                                                          \
		{l0[(n) - 2], l0[(n) - 1], l0[(n) - 2]},                                           \
		{l1[(n) - 2], l1[(n) - 1], l1[(n) - 2]},                                           \
		{l2[(n) - 2], l2[(n) - 1], l2[(n) - 2]},                                           \
	}

static inline void mpix_rggb8_to_rgb24_3x3(const uint8_t rgr0[3], const uint8_t gbg1[3],
					   const uint8_t rgr2[3], uint8_t rgb24[3])
{
	rgb24[0] = ((uint16_t)rgr0[0] + rgr0[2] + rgr2[0] + rgr2[2]) / 4;
	rgb24[1] = ((uint16_t)rgr0[1] + gbg1[2] + gbg1[0] + rgr2[1]) / 4;
	rgb24[2] = gbg1[1];
}

static inline void mpix_bggr8_to_rgb24_3x3(const uint8_t bgb0[3], const uint8_t grg1[3],
					   const uint8_t bgb2[3], uint8_t rgb24[3])
{
	rgb24[0] = grg1[1];
	rgb24[1] = ((uint16_t)bgb0[1] + grg1[2] + grg1[0] + bgb2[1]) / 4;
	rgb24[2] = ((uint16_t)bgb0[0] + bgb0[2] + bgb2[0] + bgb2[2]) / 4;
}

static inline void mpix_grbg8_to_rgb24_3x3(const uint8_t grg0[3], const uint8_t bgb1[3],
					   const uint8_t grg2[3], uint8_t rgb24[3])
{
	rgb24[0] = ((uint16_t)grg0[1] + grg2[1]) / 2;
	rgb24[1] = bgb1[1];
	rgb24[2] = ((uint16_t)bgb1[0] + bgb1[2]) / 2;
}

static inline void mpix_gbrg8_to_rgb24_3x3(const uint8_t gbg0[3], const uint8_t rgr1[3],
					   const uint8_t gbg2[3], uint8_t rgb24[3])
{
	rgb24[0] = ((uint16_t)rgr1[0] + rgr1[2]) / 2;
	rgb24[1] = rgr1[1];
	rgb24[2] = ((uint16_t)gbg0[1] + gbg2[1]) / 2;
}

static inline void mpix_rggb8_to_rgb24_2x2(uint8_t r0, uint8_t g0, uint8_t g1, uint8_t b0,
					   uint8_t rgb24[3])
{
	rgb24[0] = r0;
	rgb24[1] = ((uint16_t)g0 + g1) / 2;
	rgb24[2] = b0;
}

static inline void mpix_gbrg8_to_rgb24_2x2(uint8_t g1, uint8_t b0, uint8_t r0, uint8_t g0,
					   uint8_t rgb24[3])
{
	rgb24[0] = r0;
	rgb24[1] = ((uint16_t)g0 + g1) / 2;
	rgb24[2] = b0;
}

static inline void mpix_bggr8_to_rgb24_2x2(uint8_t b0, uint8_t g0, uint8_t g1, uint8_t r0,
					   uint8_t rgb24[3])
{
	rgb24[0] = r0;
	rgb24[1] = ((uint16_t)g0 + g1) / 2;
	rgb24[2] = b0;
}

static inline void mpix_grbg8_to_rgb24_2x2(uint8_t g1, uint8_t r0, uint8_t b0, uint8_t g0,
					   uint8_t rgb24[3])
{
	rgb24[0] = r0;
	rgb24[1] = ((uint16_t)g0 + g1) / 2;
	rgb24[2] = b0;
}

__attribute__((weak))
void mpix_convert_rggb8_to_rgb24_3x3(const uint8_t *i0, const uint8_t *i1, const uint8_t *i2,
					    uint8_t *o0, uint16_t w)
{
	uint8_t il[3][3] = FOLD_L_3X3(i0, i1, i2);
	uint8_t ir[3][3] = FOLD_R_3X3(i0, i1, i2, w);

	assert(w >= 4 && w % 2 == 0);

	mpix_grbg8_to_rgb24_3x3(il[0], il[1], il[2], &o0[0]);
	for (size_t i = 0, o = 3; i + 4 <= w; i += 2, o += 6) {
		mpix_rggb8_to_rgb24_3x3(&i0[i + 0], &i1[i + 0], &i2[i + 0], &o0[o + 0]);
		mpix_grbg8_to_rgb24_3x3(&i0[i + 1], &i1[i + 1], &i2[i + 1], &o0[o + 3]);
	}
	mpix_rggb8_to_rgb24_3x3(ir[0], ir[1], ir[2], &o0[w * 3 - 3]);
}

__attribute__((weak))
void mpix_convert_grbg8_to_rgb24_3x3(const uint8_t *i0, const uint8_t *i1, const uint8_t *i2,
					    uint8_t *o0, uint16_t w)
{
	uint8_t il[3][4] = FOLD_L_3X3(i0, i1, i2);
	uint8_t ir[3][4] = FOLD_R_3X3(i0, i1, i2, w);

	assert(w >= 4 && w % 2 == 0);

	mpix_rggb8_to_rgb24_3x3(il[0], il[1], il[2], &o0[0]);
	for (size_t i = 0, o = 3; i + 4 <= w; i += 2, o += 6) {
		mpix_grbg8_to_rgb24_3x3(&i0[i + 0], &i1[i + 0], &i2[i + 0], &o0[o + 0]);
		mpix_rggb8_to_rgb24_3x3(&i0[i + 1], &i1[i + 1], &i2[i + 1], &o0[o + 3]);
	}
	mpix_grbg8_to_rgb24_3x3(ir[0], ir[1], ir[2], &o0[w * 3 - 3]);
}

__attribute__((weak))
void mpix_convert_bggr8_to_rgb24_3x3(const uint8_t *i0, const uint8_t *i1, const uint8_t *i2,
					    uint8_t *o0, uint16_t w)
{
	uint8_t il[3][4] = FOLD_L_3X3(i0, i1, i2);
	uint8_t ir[3][4] = FOLD_R_3X3(i0, i1, i2, w);

	assert(w >= 4 && w % 2 == 0);

	mpix_gbrg8_to_rgb24_3x3(il[0], il[1], il[2], &o0[0]);
	for (size_t i = 0, o = 3; i + 4 <= w; i += 2, o += 6) {
		mpix_bggr8_to_rgb24_3x3(&i0[i + 0], &i1[i + 0], &i2[i + 0], &o0[o + 0]);
		mpix_gbrg8_to_rgb24_3x3(&i0[i + 1], &i1[i + 1], &i2[i + 1], &o0[o + 3]);
	}
	mpix_bggr8_to_rgb24_3x3(ir[0], ir[1], ir[2], &o0[w * 3 - 3]);
}

__attribute__((weak))
void mpix_convert_gbrg8_to_rgb24_3x3(const uint8_t *i0, const uint8_t *i1, const uint8_t *i2,
					    uint8_t *o0, uint16_t w)
{
	uint8_t il[3][4] = FOLD_L_3X3(i0, i1, i2);
	uint8_t ir[3][4] = FOLD_R_3X3(i0, i1, i2, w);

	assert(w >= 4 && w % 2 == 0);

	mpix_bggr8_to_rgb24_3x3(il[0], il[1], il[2], &o0[0]);
	for (size_t i = 0, o = 3; i + 4 <= w; i += 2, o += 6) {
		mpix_gbrg8_to_rgb24_3x3(&i0[i + 0], &i1[i + 0], &i2[i + 0], &o0[o + 0]);
		mpix_bggr8_to_rgb24_3x3(&i0[i + 1], &i1[i + 1], &i2[i + 1], &o0[o + 3]);
	}
	mpix_gbrg8_to_rgb24_3x3(ir[0], ir[1], ir[2], &o0[w * 3 - 3]);
}

__attribute__((weak))
void mpix_convert_rggb8_to_rgb24_2x2(const uint8_t *i0, const uint8_t *i1, uint8_t *o0,
					    uint16_t w)
{
	assert(w >= 2 && w % 2 == 0);

	for (size_t i = 0, o = 0; i + 3 <= w; i += 2, o += 6) {
		mpix_rggb8_to_rgb24_2x2(i0[i + 0], i0[i + 1], i1[i + 0], i1[i + 1], &o0[o + 0]);
		mpix_grbg8_to_rgb24_2x2(i0[i + 1], i0[i + 2], i1[i + 1], i1[i + 2], &o0[o + 3]);
	}
	mpix_rggb8_to_rgb24_2x2(i0[w - 1], i0[w - 2], i1[w - 1], i1[w - 2], &o0[w * 3 - 6]);
	mpix_grbg8_to_rgb24_2x2(i0[w - 2], i0[w - 1], i1[w - 2], i1[w - 1], &o0[w * 3 - 3]);
}

__attribute__((weak))
void mpix_convert_gbrg8_to_rgb24_2x2(const uint8_t *i0, const uint8_t *i1, uint8_t *o0,
					    uint16_t w)
{
	assert(w >= 2 && w % 2 == 0);

	for (size_t i = 0, o = 0; i + 3 <= w; i += 2, o += 6) {
		mpix_gbrg8_to_rgb24_2x2(i0[i + 0], i0[i + 1], i1[i + 0], i1[i + 1], &o0[o + 0]);
		mpix_bggr8_to_rgb24_2x2(i0[i + 1], i0[i + 2], i1[i + 1], i1[i + 2], &o0[o + 3]);
	}
	mpix_gbrg8_to_rgb24_2x2(i0[w - 1], i0[w - 2], i1[w - 1], i1[w - 2], &o0[w * 3 - 6]);
	mpix_bggr8_to_rgb24_2x2(i0[w - 2], i0[w - 1], i1[w - 2], i1[w - 1], &o0[w * 3 - 3]);
}

__attribute__((weak))
void mpix_convert_bggr8_to_rgb24_2x2(const uint8_t *i0, const uint8_t *i1, uint8_t *o0,
					    uint16_t w)
{
	assert(w >= 2 && w % 2 == 0);

	for (size_t i = 0, o = 0; i + 3 <= w; i += 2, o += 6) {
		mpix_bggr8_to_rgb24_2x2(i0[i + 0], i0[i + 1], i1[i + 0], i1[i + 1], &o0[o + 0]);
		mpix_gbrg8_to_rgb24_2x2(i0[i + 1], i0[i + 2], i1[i + 1], i1[i + 2], &o0[o + 3]);
	}
	mpix_bggr8_to_rgb24_2x2(i0[w - 1], i0[w - 2], i1[w - 1], i1[w - 2], &o0[w * 3 - 6]);
	mpix_gbrg8_to_rgb24_2x2(i0[w - 2], i0[w - 1], i1[w - 2], i1[w - 1], &o0[w * 3 - 3]);
}

__attribute__((weak))
void mpix_convert_grbg8_to_rgb24_2x2(const uint8_t *i0, const uint8_t *i1, uint8_t *o0,
					    uint16_t w)
{
	assert(w >= 2 && w % 2 == 0);

	for (size_t i = 0, o = 0; i + 3 <= w; i += 2, o += 6) {
		mpix_grbg8_to_rgb24_2x2(i0[i + 0], i0[i + 1], i1[i + 0], i1[i + 1], &o0[o + 0]);
		mpix_rggb8_to_rgb24_2x2(i0[i + 1], i0[i + 2], i1[i + 1], i1[i + 2], &o0[o + 3]);
	}
	mpix_grbg8_to_rgb24_2x2(i0[w - 1], i0[w - 2], i1[w - 1], i1[w - 2], &o0[w * 3 - 6]);
	mpix_rggb8_to_rgb24_2x2(i0[w - 2], i0[w - 1], i1[w - 2], i1[w - 1], &o0[w * 3 - 3]);
}

typedef void fn_3x3_t(const uint8_t *i0, const uint8_t *i1, const uint8_t *i2, uint8_t *o0,
		      uint16_t width);

typedef void fn_2x2_t(const uint8_t *i0, const uint8_t *i1, uint8_t *o0, uint16_t width);

static inline void mpix_op_bayer_to_rgb24_3x3(struct mpix_op *op, fn_3x3_t *fn0, fn_3x3_t *fn1)
{
	uint16_t prev_convert_offset = op->line_offset;
	const uint8_t *i0 = mpix_op_get_input_line(op);
	const uint8_t *i1 = mpix_op_peek_input_line(op);
	const uint8_t *i2 = mpix_op_peek_input_line(op);

	if (prev_convert_offset == 0) {
		fn1(i1, i0, i1, mpix_op_get_output_line(op), op->width);
		mpix_op_done(op);
	}

	if (prev_convert_offset % 2 == 0) {
		fn0(i0, i1, i2, mpix_op_get_output_line(op), op->width);
		mpix_op_done(op);
	} else {
		fn1(i0, i1, i2, mpix_op_get_output_line(op), op->width);
		mpix_op_done(op);
	}

	if (op->line_offset + 2 == op->height) {
		fn0(i1, i2, i1, mpix_op_get_output_line(op), op->width);
		mpix_op_done(op);

		/* Skip the two lines of lookahead context, now that the conversion is complete */
		mpix_op_get_input_line(op);
		mpix_op_get_input_line(op);
	}
}

static inline void mpix_op_bayer_to_rgb24_2x2(struct mpix_op *op, fn_2x2_t *fn0, fn_2x2_t *fn1)
{
	uint16_t prev_convert_offset = op->line_offset;
	const uint8_t *i0 = mpix_op_get_input_line(op);
	const uint8_t *i1 = mpix_op_peek_input_line(op);

	if (prev_convert_offset % 2 == 0) {
		fn0(i0, i1, mpix_op_get_output_line(op), op->width);
		mpix_op_done(op);
	} else {
		fn1(i0, i1, mpix_op_get_output_line(op), op->width);
		mpix_op_done(op);
	}

	if (op->line_offset + 1 == op->height) {
		fn0(i0, i1, mpix_op_get_output_line(op), op->width);
		mpix_op_done(op);

		/* Skip the two lines of lookahead context, now that the conversion is complete */
		mpix_op_get_input_line(op);
	}
}

static void mpix_op_srggb8_to_rgb24_3x3(struct mpix_op *op)
{
	mpix_op_bayer_to_rgb24_3x3(op, &mpix_convert_rggb8_to_rgb24_3x3,
				   &mpix_convert_gbrg8_to_rgb24_3x3);
}
MPIX_REGISTER_BAYER_OP(srggb8_3x3, mpix_op_srggb8_to_rgb24_3x3, SRGGB8, 3);

static void mpix_op_sgbrg8_to_rgb24_3x3(struct mpix_op *op)
{
	mpix_op_bayer_to_rgb24_3x3(op, &mpix_convert_gbrg8_to_rgb24_3x3,
				   &mpix_convert_rggb8_to_rgb24_3x3);
}
MPIX_REGISTER_BAYER_OP(sgbrg8_3x3, mpix_op_sgbrg8_to_rgb24_3x3, SGBRG8, 3);

static void mpix_op_sbggr8_to_rgb24_3x3(struct mpix_op *op)
{
	mpix_op_bayer_to_rgb24_3x3(op, &mpix_convert_bggr8_to_rgb24_3x3,
				   &mpix_convert_grbg8_to_rgb24_3x3);
}
MPIX_REGISTER_BAYER_OP(sbggr8_3x3, mpix_op_sbggr8_to_rgb24_3x3, SBGGR8, 3);

static void mpix_op_sgrbg8_to_rgb24_3x3(struct mpix_op *op)
{
	mpix_op_bayer_to_rgb24_3x3(op, &mpix_convert_grbg8_to_rgb24_3x3,
				   &mpix_convert_bggr8_to_rgb24_3x3);
}
MPIX_REGISTER_BAYER_OP(sgrbg8_3x3, mpix_op_sgrbg8_to_rgb24_3x3, SGRBG8, 3);

static void mpix_op_srggb8_to_rgb24_2x2(struct mpix_op *op)
{
	mpix_op_bayer_to_rgb24_2x2(op, &mpix_convert_rggb8_to_rgb24_2x2,
				   &mpix_convert_gbrg8_to_rgb24_2x2);
}
MPIX_REGISTER_BAYER_OP(srggb8_2x2, mpix_op_srggb8_to_rgb24_2x2, SRGGB8, 2);

static void mpix_op_sgbrg8_to_rgb24_2x2(struct mpix_op *op)
{
	mpix_op_bayer_to_rgb24_2x2(op, &mpix_convert_gbrg8_to_rgb24_2x2,
				   &mpix_convert_rggb8_to_rgb24_2x2);
}
MPIX_REGISTER_BAYER_OP(sgbrg8_2x2, mpix_op_sgbrg8_to_rgb24_2x2, SGBRG8, 2);

static void mpix_op_sbggr8_to_rgb24_2x2(struct mpix_op *op)
{
	mpix_op_bayer_to_rgb24_2x2(op, &mpix_convert_bggr8_to_rgb24_2x2,
				   &mpix_convert_grbg8_to_rgb24_2x2);
}
MPIX_REGISTER_BAYER_OP(sbggr8_2x2, mpix_op_sbggr8_to_rgb24_2x2, SBGGR8, 2);

static void mpix_op_sgrbg8_to_rgb24_2x2(struct mpix_op *op)
{
	mpix_op_bayer_to_rgb24_2x2(op, &mpix_convert_grbg8_to_rgb24_2x2,
				   mpix_convert_bggr8_to_rgb24_2x2);
}
MPIX_REGISTER_BAYER_OP(sgrbg8_2x2, mpix_op_sgrbg8_to_rgb24_2x2, SGRBG8, 2);

static const struct mpix_op **mpix_bayer_op_list = (const struct mpix_op *[]){
	MPIX_LIST_BAYER_OP
};
