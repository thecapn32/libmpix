/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_op_correction_h mpix/op_correction.h
 * @brief Low-level image correction operations
 * @{
 */
#ifndef MPIX_OP_CORRECTION_H
#define MPIX_OP_CORRECTION_H

#include <stdint.h>
#include <stdlib.h>

#include <mpix/op.h>

/**
 * @brief Amount of scaling applied to the red_level and blue_level values.
 */
#define MPIX_CORRECTION_WB_SCALE 1024

/**
 * Configuration of the image correction levels.
 */
struct mpix_correction {
	/** Offset removed to every pixel, 0 for no correction, 255 for dark image */
	uint8_t black_level;
	/** Red value correction level multiplied by 1024: 2048 applies 2x to red channe */
	uint16_t red_level;
	/** Blue value correction level multiplied by 1024: 2048 applies 2x to blue channel */
	uint16_t blue_level;
	/** Gamma level to be applied to the pixels */
	uint8_t gamma_level;
};

/**
 * Correction types that can be applied to the image.
 */
enum mpix_correction_type {
	/** Correct the black level applied to every pixel */
	MPIX_CORRECTION_BLACK_LEVEL,
	/** Apply white balance to control the red and blue channels gain (green unchanged) */
	MPIX_CORRECTION_WHITE_BALANCE,
	/** Apply gamma correction to non-linearly increase the brightness of the image */
	MPIX_CORRECTION_GAMMA,
};

/**
 * Image correction operation
 * @internal
 */
struct mpix_correction_op {
	/** Fields common to all operations. */
	struct mpix_base_op base;
	/** Type of correction operation */
	enum mpix_correction_type type;
	/** Function to convert one line of pixels */
	void (*correction_fn)(const uint8_t *src, uint8_t *dst,
			      uint16_t width, uint16_t line_offset, struct mpix_correction *corr);
	/** Storage for the control level, when when a runtime control needed */
	struct mpix_correction *correction;
};

/**
 * @brief Define a new Correction operation.
 *
 * @param id Short identifier to differentiate operations of the same type.
 * @param fn Function converting 3 input lines into 1 output line.
 * @param t Correction operation type from @ref mpix_correction_type
 * @param fmt The input format for that operation.
 */
#define MPIX_REGISTER_CORRECTION_OP(id, fn, t, fmt)                                                \
	const struct mpix_correction_op mpix_correction_op_##id = {                                \
		.base.name = ("correction_" #id),                                                  \
		.base.fourcc_src = (MPIX_FMT_##fmt),                                               \
		.base.fourcc_dst = (MPIX_FMT_##fmt),                                               \
		.base.window_size = 1,                                                             \
		.base.run = mpix_correction_op,                                                    \
		.correction_fn = (fn),                                                             \
		.type = (MPIX_CORRECTION_##t),                                                     \
	}

/**
 * @brief Correct the black level of an input line in any 8-bit RGB or grayscale pixel format.
 *
 * @param src Input buffers to convert.
 * @param dst Output line buffer receiving the conversion result.
 * @param width Width of the input and output lines in pixels.
 * @param line_offset Number of the line being processed within the frame.
 * @param corr Correction levels to apply to the image.
 */
void mpix_correction_black_level_raw8(const uint8_t *src, uint8_t *dst, uint16_t width,
				      uint16_t line_offset, struct mpix_correction *corr);
/**
 * @brief Correct the black level of an input line in RGB24 pixel format.
 * @copydetails mpix_correction_black_level_raw8
 */
void mpix_correction_black_level_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width,
				       uint16_t line_offset, struct mpix_correction *corr);
/**
 * @brief Correct the white balance of an input line in RGB24 pixel format.
 * @copydetails mpix_correction_black_level_raw8
 */
void mpix_correction_white_balance_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width,
					 uint16_t line_offset, struct mpix_correction *corr);

/**
 * Helper to simplify the implementation of a image correction operation.
 *
 * @internal
 *
 * @param base Base operation type, casted to @ref mpix_correction_op.
 */
void mpix_correction_op(struct mpix_base_op *base);

#endif /** @} */
