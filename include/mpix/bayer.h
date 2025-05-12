/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_bayer mpix/bayer.h
 * @brief Implementing debayer operations
 * @{
 */
#ifndef MPIX_BAYER_H
#define MPIX_BAYER_H

#include <stdint.h>
#include <stdlib.h>

#include <mpix/operation.h>

/**
 * @brief Define a new bayer format conversion operation.
 *
 * @param fn Function performing the operation.
 * @param fmt_in The input format for that operation.
 * @param win_sz The number of line of context needed for that debayer operation.
 */
#define MPIX_DEFINE_BAYER_OPERATION(fn, fmt_in, win_sz)                                            \
	static const STRUCT_SECTION_ITERABLE_ALTERNATE(mpix_convert, mpix_operation, fn##_op) = {  \
		.name = #fn,                                                                       \
		.format_in = (MPIX_FORMAT_##fmt_in),                                               \
		.format_out = MPIX_FORMAT_RGB24,                                                   \
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
void mpix_line_rggb8_to_rgb24_3x3(const uint8_t *i0, const uint8_t *i1, const uint8_t *i2,
				  uint8_t *rgb24, uint16_t width);
/**
 * @brief Convert a line from GRBG8 to RGB24 with 3x3 method
 * @copydetails mpix_line_rggb8_to_rgb24_3x3()
 */
void mpix_line_grbg8_to_rgb24_3x3(const uint8_t *i0, const uint8_t *i1, const uint8_t *i2,
				  uint8_t *rgb24, uint16_t width);
/**
 * @brief Convert a line from BGGR8 to RGB24 with 3x3 method
 * @copydetails mpix_line_rggb8_to_rgb24_3x3()
 */
void mpix_line_bggr8_to_rgb24_3x3(const uint8_t *i0, const uint8_t *i1, const uint8_t *i2,
				  uint8_t *rgb24, uint16_t width);
/**
 * @brief Convert a line from GBRG8 to RGB24 with 3x3 method
 * @copydetails mpix_line_rggb8_to_rgb24_3x3()
 */
void mpix_line_gbrg8_to_rgb24_3x3(const uint8_t *i0, const uint8_t *i1, const uint8_t *i2,
				  uint8_t *rgb24, uint16_t width);

/**
 * @brief Convert a line from RGGB8 to RGB24 with 2x2 method
 *
 * @param i0 Buffer of the input row number 0 in bayer format (1 byte per pixel).
 * @param i1 Buffer of the input row number 1 in bayer format (1 byte per pixel).
 * @param rgb24 Buffer of the output row in RGB24 format (3 bytes per pixel).
 * @param width Width of the lines in number of pixels.
 */
void mpix_line_rggb8_to_rgb24_2x2(const uint8_t *i0, const uint8_t *i1, uint8_t *rgb24,
				  uint16_t width);
/**
 * @brief Convert a line from GBRG8 to RGB24 with 2x2 method
 * @copydetails mpix_line_rggb8_to_rgb24_2x2()
 */
void mpix_line_gbrg8_to_rgb24_2x2(const uint8_t *i0, const uint8_t *i1, uint8_t *rgb24,
				  uint16_t width);
/**
 * @brief Convert a line from BGGR8 to RGB24 with 2x2 method
 * @copydetails mpix_line_rggb8_to_rgb24_2x2()
 */
void mpix_line_bggr8_to_rgb24_2x2(const uint8_t *i0, const uint8_t *i1, uint8_t *rgb24,
				  uint16_t width);
/**
 * @brief Convert a line from GRBG8 to RGB24 with 2x2 method
 * @copydetails mpix_line_rggb8_to_rgb24_2x2()
 */
void mpix_line_grbg8_to_rgb24_2x2(const uint8_t *i0, const uint8_t *i1, uint8_t *rgb24,
				  uint16_t width);

#endif /** @} */
