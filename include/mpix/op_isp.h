/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_op_isp mpix/op_isp.h
 * @brief Low-level Image Signal Processing (ISP) operations
 * @{
 */
#ifndef MPIX_OP_ISP_H
#define MPIX_OP_ISP_H

#include <stdint.h>
#include <stdlib.h>

#include <mpix/op.h>

struct mpix_isp {
	/** Offset removed to every pixel, 0 for no correction, 255 for dark image */
	uint8_t black_level;
	/** Red value correction level, 0 for unchanged, 255 for max red */
	uint8_t red_level;
	/** Blue value correction level, 0 for unchanged, 255 for max blue */
	uint8_t blue_level;
	/** Gamma level to be applied to the pixels */
	uint8_t gamma_level;
};

/**
 * Available isp operations to apply to the image.
 */
enum mpix_isp_type {
	/** Correct the black level applied to every pixel */
	MPIX_ISP_BLACK_LEVEL,
};

/** @internal */
struct mpix_isp_op {
	/** Fields common to all operations. */
	struct mpix_base_op base;
	/** Type of isp operation */
	enum mpix_isp_type type;
	/** Function to convert one line of pixels */
	void (*isp_fn)(const uint8_t *src, uint8_t *dst, uint16_t width, struct mpix_isp *isp);
	/** Storage for the control level, when when a runtime control needed */
	struct mpix_isp *isp;
};

/**
 * @brief Define a new ISP operation.
 *
 * @param id Short identifier to differentiate operations of the same type.
 * @param fn Function converting 3 input lines into 1 output line.
 * @param t ISP operation type from @ref mpix_isp_type
 * @param fmt The input format for that operation.
 */
#define MPIX_REGISTER_ISP_OP(id, fn, t, fmt)                                                       \
	const struct mpix_isp_op mpix_isp_op_##id = {                                              \
		.base.name = ("isp_" #id),                                                         \
		.base.format_src = (MPIX_FMT_##fmt),                                               \
		.base.format_dst = (MPIX_FMT_##fmt),                                               \
		.base.window_size = 1,                                                             \
		.base.run = mpix_isp_op,                                                           \
		.isp_fn = (fn),                                                                    \
		.type = (MPIX_ISP_##t),                                                            \
	}

/**
 * @brief Correct the black level of an input line in any 8-bit RGB or grayscale pixel format.
 *
 * @param src Input buffers to convert.
 * @param dst Output line buffer receiving the conversion result.
 * @param width Width of the input and output lines in pixels.
 * @param isp ISP context holding the current black level value.
 */
void mpix_isp_black_level_raw8(const uint8_t *src, uint8_t *dst, uint16_t size,
			       struct mpix_isp *isp);
/**
 * @brief Correct the black level of an input line in RGB24 pixel format.
 * @copydetails mpix_isp_black_level_raw8
 */
void mpix_isp_black_level_rgb24(const uint8_t *src, uint8_t *dst, uint16_t size,
			        struct mpix_isp *isp);

/** @internal */
void mpix_isp_op(struct mpix_base_op *base);

#endif /** @} */
