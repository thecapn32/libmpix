/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_op_bayer mpix/op_bayer.h
 * @brief Implementing new debayer operations
 * @{
 */
#ifndef MPIX_OP_BAYER_H
#define MPIX_OP_BAYER_H

#include <stdint.h>
#include <stdlib.h>

#include <mpix/op.h>

/**
 * @brief Define a new bayer format conversion operation.
 *
 * @param id Short identifier to differentiate operations of the same type.
 * @param fn Function performing the operation.
 * @param fmt_in The input format for that operation.
 * @param win_sz The number of line of context needed for that debayer operation.
 */
#define MPIX_REGISTER_BAYER_OP(id, fn, fmt_in, win_sz)                                             \
	const struct mpix_op mpix_bayer_op_##id = {                                                \
		.name = ("bayer_" #id),                                                            \
		.format_in = (MPIX_FMT_##fmt_in),                                                  \
		.format_out = MPIX_FMT_RGB24,                                                      \
		.window_size = (win_sz),                                                           \
		.run = (fn),                                                                       \
	}

/**
 * @brief Convert a line from RGGB8 to RGB24 with 3x3 method
 *
 * @param i0 Buffer of the input row number 0 in bayer format (1 byte per pixel).
 * @param i1 Buffer of the input row number 1 in bayer format (1 byte per pixel).
 * @param i2 Buffer of the input row number 2 in bayer format (1 byte per pixel).
 * @param rgb24 Buffer of the output row in RGB24 format (3 bytes per pixel).
 * @param width Width of the lines in number of pixels.
 */
void mpix_convert_rggb8_to_rgb24_3x3(const uint8_t *i0, const uint8_t *i1, const uint8_t *i2,
				     uint8_t *rgb24, uint16_t width);
/**
 * @brief Convert a line from GRBG8 to RGB24 with 3x3 method
 * @copydetails mpix_convert_rggb8_to_rgb24_3x3()
 */
void mpix_convert_grbg8_to_rgb24_3x3(const uint8_t *i0, const uint8_t *i1, const uint8_t *i2,
				     uint8_t *rgb24, uint16_t width);
/**
 * @brief Convert a line from BGGR8 to RGB24 with 3x3 method
 * @copydetails mpix_convert_rggb8_to_rgb24_3x3()
 */
void mpix_convert_bggr8_to_rgb24_3x3(const uint8_t *i0, const uint8_t *i1, const uint8_t *i2,
				     uint8_t *rgb24, uint16_t width);
/**
 * @brief Convert a line from GBRG8 to RGB24 with 3x3 method
 * @copydetails mpix_convert_rggb8_to_rgb24_3x3()
 */
void mpix_convert_gbrg8_to_rgb24_3x3(const uint8_t *i0, const uint8_t *i1, const uint8_t *i2,
				     uint8_t *rgb24, uint16_t width);

/**
 * @brief Convert a line from RGGB8 to RGB24 with 2x2 method
 *
 * @param i0 Buffer of the input row number 0 in bayer format (1 byte per pixel).
 * @param i1 Buffer of the input row number 1 in bayer format (1 byte per pixel).
 * @param rgb24 Buffer of the output row in RGB24 format (3 bytes per pixel).
 * @param width Width of the lines in number of pixels.
 */
void mpix_convert_rggb8_to_rgb24_2x2(const uint8_t *i0, const uint8_t *i1, uint8_t *rgb24,
				     uint16_t width);
/**
 * @brief Convert a line from GBRG8 to RGB24 with 2x2 method
 * @copydetails mpix_convert_rggb8_to_rgb24_2x2()
 */
void mpix_convert_gbrg8_to_rgb24_2x2(const uint8_t *i0, const uint8_t *i1, uint8_t *rgb24,
				     uint16_t width);
/**
 * @brief Convert a line from BGGR8 to RGB24 with 2x2 method
 * @copydetails mpix_convert_rggb8_to_rgb24_2x2()
 */
void mpix_convert_bggr8_to_rgb24_2x2(const uint8_t *i0, const uint8_t *i1, uint8_t *rgb24,
				     uint16_t width);
/**
 * @brief Convert a line from GRBG8 to RGB24 with 2x2 method
 * @copydetails mpix_convert_rggb8_to_rgb24_2x2()
 */
void mpix_convert_grbg8_to_rgb24_2x2(const uint8_t *i0, const uint8_t *i1, uint8_t *rgb24,
				     uint16_t width);

#endif /** @} */
