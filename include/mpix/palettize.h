/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_resize mpix/resize.h
 * @brief Implementing palettization operations
 * @{
 */
#ifndef MPIX_PALETTIZE_H
#define MPIX_PALETTIZE_H

#include <stdlib.h>
#include <stdint.h>

#include <zephyr/sys/util.h>
#include <zephyr/sys/byteorder.h>
#include <mpix/image.h>

/**
 * @brief Color palette as a list of pixels in the described format.
 */
struct mpix_palette {
	/** Array of pixels whose position (in pixel) is the inxdex  */
	uint8_t *colors;
	/** Nuber of pixels in the palette */
	uint16_t size;
	/** Format of each pixel in the palette */
	uint32_t format;
};

/**
 * @brief Define a new palettization operation: from a pixel format to indexed colors.
 *
 * @param fn Function converting one input line.
 * @param fmt_in The input format for that operation.
 * @param fmt_out The Output format for that operation.
 */
#define MPIX_DEFINE_PALETTIZE_OPERATION(fn, fmt_in, fmt_out)                                       \
	static const STRUCT_SECTION_ITERABLE_ALTERNATE(mpix_convert, mpix_operation, fn##_op) = {  \
		.name = #fn,                                                                       \
		.format_in = (MPIX_FORMAT_##fmt_in),                                               \
		.format_out = (MPIX_FORMAT_##fmt_out),                                             \
		.window_size = 1,                                                                  \
		.run = mpix_palettize_op,                                                          \
		.arg0 = (fn),                                                                      \
	}

/**
 * @brief Define a new palettization operation: from indexed colors to different pixel format.
 * @copydetails MPIX_DEFINE_PALETTIZE_OPERATION
 */
#define MPIX_DEFINE_DEPALETTIZE_OPERATION(fn, fmt_in, fmt_out)                                     \
	MPIX_DEFINE_PALETTIZE_OPERATION(fn, fmt_in, fmt_out)

/**
 * @brief Helper to turn a line palettization function into an operation.
 *
 * The line conversion function is to be provided in @c op->arg.
 * It processes on the input line to convert it to the destination format.
 *
 * The palette is to be provided in @c op->arg1.
 *
 * @param op Current operation in progress.
 */
void mpix_palettize_op(struct mpix_operation *op);

/**
 * @brief Convert a line of pixel data from RGB24 to PALETTE8.
 *
 * You only need to call this function to work directly on raw buffers.
 * See @ref mpix_image_palettize for a more convenient high-level API.
 *
 * @param src Buffer of the input line, with the format @c XXX in @c mpix_line_XXX_to_YYY().
 * @param dst Buffer of the output line, with the format @c YYY in @c mpix_line_XXX_to_YYY().
 * @param width Width of the lines in number of pixels.
 * @param palette Color palette to use for the conversion.
 */
void mpix_line_rgb24_to_palette8(const uint8_t *src, uint8_t *dst, uint16_t width,
				 const struct mpix_palette *palette);
/**
 * @brief Convert a line of pixel data from PALETTE8 to RGB24.
 * @copydetails mpix_line_rgb24_to_palette8
 */
void mpix_line_palette8_to_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width,
				  const struct mpix_palette *palette);
/**
 * @brief Convert a line of pixel data from RGB24 to PALETTE4.
 * @copydetails mpix_line_rgb24_to_palette8
 */
void mpix_line_rgb24_to_palette4(const uint8_t *src, uint8_t *dst, uint16_t width,
				  const struct mpix_palette *palette);
/**
 * @brief Convert a line of pixel data from PALETTE4 to RGB24.
 * @copydetails mpix_line_rgb24_to_palette8
 */
void mpix_line_palette4_to_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width,
				  const struct mpix_palette *palette);
/**
 * @brief Convert a line of pixel data from RGB24 to PALETTE2.
 * @copydetails mpix_line_rgb24_to_palette8
 */
void mpix_line_rgb24_to_palette2(const uint8_t *src, uint8_t *dst, uint16_t width,
				  const struct mpix_palette *palette);
/**
 * @brief Convert a line of pixel data from PALETTE2 to RGB24.
 * @copydetails mpix_line_rgb24_to_palette8
 */
void mpix_line_palette2_to_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width,
				  const struct mpix_palette *palette);
/**
 * @brief Convert a line of pixel data from RGB24 to PALETTE1.
 * @copydetails mpix_line_rgb24_to_palette8
 */
void mpix_line_rgb24_to_palette1(const uint8_t *src, uint8_t *dst, uint16_t width,
				  const struct mpix_palette *palette);
/**
 * @brief Convert a line of pixel data from PALETTE1 to RGB24.
 * @copydetails mpix_line_rgb24_to_palette8
 */
void mpix_line_palette1_to_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width,
				  const struct mpix_palette *palette);

#endif /** @} */
