/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_op_palettize mpix/op_palettize.h
 * @brief Implementing new palettization operations
 * @{
 */
#ifndef MPIX_OP_PALETTIZE_H
#define MPIX_OP_PALETTIZE_H

#include <stdlib.h>
#include <stdint.h>

#include <mpix/op.h>

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
 * @param id Short identifier to differentiate operations of the same category.
 * @param fn Function converting one input line.
 * @param fmt_in The input format for that operation.
 * @param fmt_out The Output format for that operation.
 */
#define MPIX_REGISTER_PALETTIZE_OP(id, fn, fmt_in, fmt_out)                                        \
	const struct mpix_op mpix_palettize_op_##id = {                                            \
		.name = ("palettize_" #id),                                                        \
		.format_in = (MPIX_FMT_##fmt_in),                                                  \
		.format_out = (MPIX_FMT_##fmt_out),                                                \
		.window_size = 1,                                                                  \
		.run = mpix_palettize_op,                                                          \
		.arg0 = (fn),                                                                      \
	}

/**
 * @brief Convert a line of pixel data from RGB24 to PALETTE8.
 *
 * You only need to call this function to work directly on raw buffers.
 * See @ref mpix_image_palettize for a more convenient high-level API.
 *
 * @param src Buffer of the input line, with the format @c XXX in @c mpix_palettize_XXX_to_YYY().
 * @param dst Buffer of the output line, with the format @c YYY in @c mpix_palettize_XXX_to_YYY().
 * @param width Width of the lines in number of pixels.
 * @param palette Color palette to use for the conversion.
 */
void mpix_convert_rgb24_to_palette8(const uint8_t *src, uint8_t *dst, uint16_t width,
				    const struct mpix_palette *palette);
/**
 * @brief Convert a line of pixel data from PALETTE8 to RGB24.
 * @copydetails mpix_convert_rgb24_to_palette8
 */
void mpix_convert_palette8_to_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width,
				    const struct mpix_palette *palette);
/**
 * @brief Convert a line of pixel data from RGB24 to PALETTE4.
 * @copydetails mpix_convert_rgb24_to_palette8
 */
void mpix_convert_rgb24_to_palette4(const uint8_t *src, uint8_t *dst, uint16_t width,
				    const struct mpix_palette *palette);
/**
 * @brief Convert a line of pixel data from PALETTE4 to RGB24.
 * @copydetails mpix_convert_rgb24_to_palette8
 */
void mpix_convert_palette4_to_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width,
				    const struct mpix_palette *palette);
/**
 * @brief Convert a line of pixel data from RGB24 to PALETTE2.
 * @copydetails mpix_convert_rgb24_to_palette8
 */
void mpix_convert_rgb24_to_palette2(const uint8_t *src, uint8_t *dst, uint16_t width,
				    const struct mpix_palette *palette);
/**
 * @brief Convert a line of pixel data from PALETTE2 to RGB24.
 * @copydetails mpix_convert_rgb24_to_palette8
 */
void mpix_convert_palette2_to_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width,
				    const struct mpix_palette *palette);
/**
 * @brief Convert a line of pixel data from RGB24 to PALETTE1.
 * @copydetails mpix_convert_rgb24_to_palette8
 */
void mpix_convert_rgb24_to_palette1(const uint8_t *src, uint8_t *dst, uint16_t width,
				    const struct mpix_palette *palette);
/**
 * @brief Convert a line of pixel data from PALETTE1 to RGB24.
 * @copydetails mpix_convert_rgb24_to_palette8
 */
void mpix_convert_palette1_to_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width,
				    const struct mpix_palette *palette);

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
void mpix_palettize_op(struct mpix_op *op);

#endif /** @} */
