/* SPDX-License-Identifier: Apache-2.0 */
#include <stddef.h>
#include <stdint.h>
#include <arm_neon.h>

#include <mpix/low_level.h>
#include <mpix/operation.h>

#define MPIX_PREFETCH_DIST 128
#define MPIX_ROUND_SHIFT 0

#define SHR_Q10(v)  vqrshrn_n_s32((v), 10)

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

size_t mpix_correct_color_matrix_raw8_neon(const uint8_t *__restrict src,
                                           uint8_t *__restrict dst,
                                           uint16_t width,
                                           const int32_t matrix_q10[9]) {
  size_t n = width & ~15u;
  if (!n) return 0;

  // Broadcast Q10 coeffs as s16x8 (safe if |coef| < 32768).
  const int16x8_t k00 = vdupq_n_s16((int16_t)matrix_q10[0]);
  const int16x8_t k01 = vdupq_n_s16((int16_t)matrix_q10[1]);
  const int16x8_t k02 = vdupq_n_s16((int16_t)matrix_q10[2]);
  const int16x8_t k10 = vdupq_n_s16((int16_t)matrix_q10[3]);
  const int16x8_t k11 = vdupq_n_s16((int16_t)matrix_q10[4]);
  const int16x8_t k12 = vdupq_n_s16((int16_t)matrix_q10[5]);
  const int16x8_t k20 = vdupq_n_s16((int16_t)matrix_q10[6]);
  const int16x8_t k21 = vdupq_n_s16((int16_t)matrix_q10[7]);
  const int16x8_t k22 = vdupq_n_s16((int16_t)matrix_q10[8]);

  const uint8_t *s = src;
  uint8_t *d = dst;

  for (size_t i = 0; i < n; i += 16) {
    __builtin_prefetch(s + MPIX_PREFETCH_DIST, 0, 1);
    __builtin_prefetch(d + MPIX_PREFETCH_DIST, 1, 1);

    // Load 16 interleaved pixels: R,G,B (uint8x16 each)
    uint8x16x3_t rgb = vld3q_u8(s);
    s += 16 * 3;

    // Widen to s16
    int16x8_t r_lo = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(rgb.val[0])));
    int16x8_t r_hi = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(rgb.val[0])));
    int16x8_t g_lo = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(rgb.val[1])));
    int16x8_t g_hi = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(rgb.val[1])));
    int16x8_t b_lo = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(rgb.val[2])));
    int16x8_t b_hi = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(rgb.val[2])));

    // -------- Row 0: R' = R*k00 + G*k01 + B*k02 --------
    int32x4_t r0_lo0 = vmull_s16(vget_low_s16 (r_lo), vget_low_s16 (k00));
    r0_lo0 = vmlal_s16(r0_lo0, vget_low_s16 (g_lo), vget_low_s16 (k01));
    r0_lo0 = vmlal_s16(r0_lo0, vget_low_s16 (b_lo), vget_low_s16 (k02));
    int32x4_t r0_lo1 = vmull_high_s16(r_lo, k00);
    r0_lo1 = vmlal_high_s16(r0_lo1, g_lo, k01);
    r0_lo1 = vmlal_high_s16(r0_lo1, b_lo, k02);

    int32x4_t r0_hi0 = vmull_s16(vget_low_s16 (r_hi), vget_low_s16 (k00));
    r0_hi0 = vmlal_s16(r0_hi0, vget_low_s16 (g_hi), vget_low_s16 (k01));
    r0_hi0 = vmlal_s16(r0_hi0, vget_low_s16 (b_hi), vget_low_s16 (k02));
    int32x4_t r0_hi1 = vmull_high_s16(r_hi, k00);
    r0_hi1 = vmlal_high_s16(r0_hi1, g_hi, k01);
    r0_hi1 = vmlal_high_s16(r0_hi1, b_hi, k02);

    int16x8_t r16_lo = vcombine_s16(SHR_Q10(r0_lo0), SHR_Q10(r0_lo1));
    int16x8_t r16_hi = vcombine_s16(SHR_Q10(r0_hi0), SHR_Q10(r0_hi1));

    // -------- Row 1: G' = R*k10 + G*k11 + B*k12 --------
    int32x4_t g1_lo0 = vmull_s16(vget_low_s16 (r_lo), vget_low_s16 (k10));
    g1_lo0 = vmlal_s16(g1_lo0, vget_low_s16 (g_lo), vget_low_s16 (k11));
    g1_lo0 = vmlal_s16(g1_lo0, vget_low_s16 (b_lo), vget_low_s16 (k12));
    int32x4_t g1_lo1 = vmull_high_s16(r_lo, k10);
    g1_lo1 = vmlal_high_s16(g1_lo1, g_lo, k11);
    g1_lo1 = vmlal_high_s16(g1_lo1, b_lo, k12);

    int32x4_t g1_hi0 = vmull_s16(vget_low_s16 (r_hi), vget_low_s16 (k10));
    g1_hi0 = vmlal_s16(g1_hi0, vget_low_s16 (g_hi), vget_low_s16 (k11));
    g1_hi0 = vmlal_s16(g1_hi0, vget_low_s16 (b_hi), vget_low_s16 (k12));
    int32x4_t g1_hi1 = vmull_high_s16(r_hi, k10);
    g1_hi1 = vmlal_high_s16(g1_hi1, g_hi, k11);
    g1_hi1 = vmlal_high_s16(g1_hi1, b_hi, k12);

    int16x8_t g16_lo = vcombine_s16(SHR_Q10(g1_lo0), SHR_Q10(g1_lo1));
    int16x8_t g16_hi = vcombine_s16(SHR_Q10(g1_hi0), SHR_Q10(g1_hi1));

    // -------- Row 2: B' = R*k20 + G*k21 + B*k22 --------
    int32x4_t b2_lo0 = vmull_s16(vget_low_s16 (r_lo), vget_low_s16 (k20));
    b2_lo0 = vmlal_s16(b2_lo0, vget_low_s16 (g_lo), vget_low_s16 (k21));
    b2_lo0 = vmlal_s16(b2_lo0, vget_low_s16 (b_lo), vget_low_s16 (k22));
    int32x4_t b2_lo1 = vmull_high_s16(r_lo, k20);
    b2_lo1 = vmlal_high_s16(b2_lo1, g_lo, k21);
    b2_lo1 = vmlal_high_s16(b2_lo1, b_lo, k22);

    int32x4_t b2_hi0 = vmull_s16(vget_low_s16 (r_hi), vget_low_s16 (k20));
    b2_hi0 = vmlal_s16(b2_hi0, vget_low_s16 (g_hi), vget_low_s16 (k21));
    b2_hi0 = vmlal_s16(b2_hi0, vget_low_s16 (b_hi), vget_low_s16 (k22));
    int32x4_t b2_hi1 = vmull_high_s16(r_hi, k20);
    b2_hi1 = vmlal_high_s16(b2_hi1, g_hi, k21);
    b2_hi1 = vmlal_high_s16(b2_hi1, b_hi, k22);

    int16x8_t b16_lo = vcombine_s16(SHR_Q10(b2_lo0), SHR_Q10(b2_lo1));
    int16x8_t b16_hi = vcombine_s16(SHR_Q10(b2_hi0), SHR_Q10(b2_hi1));

    uint8x8_t r_u8_lo = vqmovun_s16(r16_lo);
    uint8x16_t R = vqmovun_high_s16(r_u8_lo, r16_hi);
    uint8x8_t g_u8_lo = vqmovun_s16(g16_lo);
    uint8x16_t G = vqmovun_high_s16(g_u8_lo, g16_hi);
    uint8x8_t b_u8_lo = vqmovun_s16(b16_lo);
    uint8x16_t B = vqmovun_high_s16(b_u8_lo, b16_hi);

    uint8x16x3_t out;
    out.val[0] = R;
    out.val[1] = G;
    out.val[2] = B;
    vst3q_u8(d, out);
    d += 16 * 3;
  }

  return n;
}

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
