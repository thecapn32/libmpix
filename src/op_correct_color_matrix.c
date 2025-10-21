/* SPDX-License-Identifier: Apache-2.0 */

#ifdef CONFIG_MPIX_CMSIS_DSP
#include <arm_math.h>
#endif
#include <mpix/low_level.h>
#include <mpix/operation.h>

MPIX_REGISTER_OP(correct_color_matrix);

struct mpix_operation {
	struct mpix_base_op base;
	/* Controls */
	int32_t color_matrix_q10[9];
};

int mpix_add_correct_color_matrix(struct mpix_image *img, const int32_t *params)
{
	struct mpix_operation *op;
	size_t pitch = mpix_format_pitch(&img->fmt);

	(void)params;

	/* Add an operation */
	op = mpix_op_append(img, MPIX_OP_CORRECT_COLOR_MATRIX, sizeof(*op), pitch);
	if (op == NULL) return -ENOMEM;

	/* Register controls */
	img->ctrls[MPIX_CID_COLOR_MATRIX] = op->color_matrix_q10;

	return 0;
}

#ifdef CONFIG_MPIX_CMSIS_DSP
static inline q15_t u8_to_q15(uint8_t x) {
    return (q15_t)((uint16_t)x << 7);
}

static inline q15_t q10_to_q15(int32_t x) {
    return (q15_t)((int32_t) x << 5);
}

static inline uint8_t q15_to_u8(q15_t q) {
    int32_t t = (int32_t)q + 64;  // round-to-nearest
    t >>= 7;
    return (uint8_t)t;
}
#endif

void mpix_correct_color_matrix_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width,
				     int32_t matrix_q10[9])
{
	/* Run as many cycles of SIMD-accelerated code as possible */
#if CONFIG_MPIX_SIMD_NEON
	w += mpix_correct_color_matrix_raw8_neon(src, dst, width, matrix_q10);
#endif
#if CONFIG_MPIX_SIMD_HELIUM
	w += mpix_correct_color_matrix_raw8_helium(src, dst, width, matrix_q10);
#endif
#if CONFIG_MPIX_SIMD_RVV
	w += mpix_correct_color_matrix_raw8_rvv(src, dst, width, matrix_q10);
#endif
#ifdef CONFIG_MPIX_CMSIS_DSP
    q15_t cm[9];
    for (size_t i = 0; i < 9; i++) {
        cm[i] = CLAMP(q10_to_q15(matrix_q10[i]), -32767, 32767);
    }

    arm_matrix_instance_q15 c;
    arm_mat_init_q15(&c, 3, 3, cm);

    for (size_t w = 0; w + 3 <= width; w++, src += 3, dst += 3) {
        q15_t s[3];
        s[0] = u8_to_q15(src[0]);
        s[1] = u8_to_q15(src[1]);
        s[2] = u8_to_q15(src[2]);

        q15_t d[3];
        arm_mat_vec_mult_q15(&c, s, d);

        dst[0] = CLAMP(q15_to_u8(d[0]), 0x00, 0xff);
        dst[1] = CLAMP(q15_to_u8(d[1]), 0x00, 0xff);
        dst[2] = CLAMP(q15_to_u8(d[2]), 0x00, 0xff);
    }
#endif

    /* Finish with the buffer with C implementation */
    for (size_t w = 0; w + 3 <= width; w++, dst += 3, src += 3) {
		int32_t r;
		int32_t g;
		int32_t b;

		r = (src[0] * matrix_q10[0]) >> 10;
		g = (src[1] * matrix_q10[1]) >> 10;
		b = (src[2] * matrix_q10[2]) >> 10;
		dst[0] = CLAMP(r + g + b, 0x00, 0xff);

		r = (src[0] * matrix_q10[3]) >> 10;
		g = (src[1] * matrix_q10[4]) >> 10;
		b = (src[2] * matrix_q10[5]) >> 10;
		dst[1] = CLAMP(r + g + b, 0x00, 0xff);

		r = (src[0] * matrix_q10[6]) >> 10;
		g = (src[1] * matrix_q10[7]) >> 10;
		b = (src[2] * matrix_q10[8]) >> 10;
		dst[2] = CLAMP(r + g + b, 0x00, 0xff);
	}
}

int mpix_run_correct_color_matrix(struct mpix_base_op *base)
{
	struct mpix_operation *op = (void *)base;
	const uint8_t *src;
	uint8_t *dst;

	MPIX_OP_INPUT_LINES(base, &src, 1);
	MPIX_OP_OUTPUT_LINE(base, &dst);

	switch (base->fmt.fourcc) {
	case MPIX_FMT_RGB24:
		mpix_correct_color_matrix_rgb24(src, dst, base->fmt.width, op->color_matrix_q10);
		break;
	default:
		return -ENOTSUP;
	}

	MPIX_OP_OUTPUT_DONE(base);
	MPIX_OP_INPUT_DONE(base, 1);

	return 0;
}
