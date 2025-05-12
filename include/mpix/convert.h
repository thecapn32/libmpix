/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_convert mpix/convert.h
 * @brief Implementing conversion operations
 * @{
 */
#ifndef MPIX_CONVERT_H
#define MPIX_CONVERT_H

#include <stdlib.h>
#include <stdint.h>

#include <zephyr/sys/util.h>
#include <zephyr/sys/byteorder.h>
#include <mpix/image.h>

/**
 * @brief Define a new format conversion operation.
 *
 * @param fn Function converting one input line.
 * @param fmt_in The input format for that operation.
 * @param fmt_out The Output format for that operation.
 */
#define MPIX_DEFINE_CONVERT_OPERATION(fn, fmt_in, fmt_out)                                        \
	static const STRUCT_SECTION_ITERABLE_ALTERNATE(mpix_convert, mpix_operation,             \
						       fn##_op) = {                                \
		.name = #fn,                                                                       \
		.format_in = (MPIX_FORMAT_##fmt_in),                                              \
		.format_out = (MPIX_FORMAT_##fmt_out),                                            \
		.window_size = 1,                                                                  \
		.run = mpix_convert_op,                                                           \
		.arg0 = (fn),                                                                       \
	}
/**
 * @brief Helper to turn a format conversion function into an operation.
 *
 * The line conversion function is to be provided in @c op->arg0.
 * It processes on the input line to convert it to the destination format.
 *
 * @param op Current operation in progress.
 */
void mpix_convert_op(struct mpix_operation *op);

/**
 * @brief Get the luminance (luma channel) of an RGB24 pixel.
 *
 * @param rgb24 Pointer to an RGB24 pixel: red, green, blue channels.
 */
uint8_t mpix_rgb24_get_luma_bt709(const uint8_t rgb24[3]);

/**
 * @brief Convert a line of pixel data from RGB24 to RGB24 (null conversion).
 *
 * See @ref mpix_formats for the definition of the input and output formats.
 *
 * You only need to call this function to work directly on raw buffers.
 * See @ref mpix_image_convert for converting between formats.
 *
 * @param src Buffer of the input line in the format, @c XXX in @c mpix_line_XXX_to_YYY().
 * @param dst Buffer of the output line in the format, @c YYY in @c mpix_line_XXX_to_YYY().
 * @param width Width of the lines in number of pixels.
 */
void mpix_line_rgb24_to_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width);
/**
 * @brief Convert a line of pixel data from RGB332 to RGB24.
 * @copydetails mpix_line_rgb24_to_rgb24()
 */
void mpix_line_rgb332_to_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width);
/**
 * @brief Convert a line of pixel data from RGB24 to RGB332 little-endian.
 * @copydetails mpix_line_rgb24_to_rgb24()
 */
void mpix_line_rgb24_to_rgb332(const uint8_t *src, uint8_t *dst, uint16_t width);
/**
 * @brief Convert a line of pixel data from RGB565 little-endian to RGB24.
 * @copydetails mpix_line_rgb24_to_rgb24()
 */
void mpix_line_rgb565le_to_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width);
/**
 * @brief Convert a line of pixel data from RGB565 big-endian to RGB24.
 * @copydetails mpix_line_rgb24_to_rgb24()
 */
void mpix_line_rgb565be_to_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width);
/**
 * @brief Convert a line of pixel data from RGB24 to RGB565 little-endian.
 * @copydetails mpix_line_rgb24_to_rgb24()
 */
void mpix_line_rgb24_to_rgb565le(const uint8_t *src, uint8_t *dst, uint16_t width);
/**
 * @brief Convert a line of pixel data from RGB24 to RGB565 big-endian.
 * @copydetails mpix_line_rgb24_to_rgb24()
 */
void mpix_line_rgb24_to_rgb565be(const uint8_t *src, uint8_t *dst, uint16_t width);
/**
 * @brief Convert a line of pixel data from YUYV to RGB24 (BT.709 coefficients).
 * @copydetails mpix_line_rgb24_to_rgb24()
 */
void mpix_line_yuyv_to_rgb24_bt709(const uint8_t *src, uint8_t *dst, uint16_t width);
/**
 * @brief Convert a line of pixel data from RGB24 to YUYV (BT.709 coefficients).
 * @copydetails mpix_line_rgb24_to_rgb24()
 */
void mpix_line_rgb24_to_yuyv_bt709(const uint8_t *src, uint8_t *dst, uint16_t width);
/**
 * @brief Convert a line of pixel data from RGB24 to YUV24 (BT.709 coefficients).
 * @copydetails mpix_line_rgb24_to_rgb24()
 */
void mpix_line_rgb24_to_yuv24_bt709(const uint8_t *src, uint8_t *dst, uint16_t width);
/**
 * @brief Convert a line of pixel data from YUV24 to RGB24 (BT.709 coefficients).
 * @copydetails mpix_line_rgb24_to_rgb24()
 */
void mpix_line_yuv24_to_rgb24_bt709(const uint8_t *src, uint8_t *dst, uint16_t width);
/**
 * @brief Convert a line of pixel data from YUYV to YUV24
 * @copydetails mpix_line_rgb24_to_rgb24()
 */
void mpix_line_yuyv_to_yuv24(const uint8_t *src, uint8_t *dst, uint16_t width);
/**
 * @brief Convert a line of pixel data from YUV24 to YUYV
 * @copydetails mpix_line_rgb24_to_rgb24()
 */
void mpix_line_yuv24_to_yuyv(const uint8_t *src, uint8_t *dst, uint16_t width);
/**
 * @brief Convert a line of pixel data from Y8 to RGB24
 * @copydetails mpix_line_rgb24_to_rgb24()
 */
void mpix_line_y8_to_rgb24_bt709(const uint8_t *src, uint8_t *dst, uint16_t width);
/**
 * @brief Convert a line of pixel data from RGB24 to Y8
 * @copydetails mpix_line_rgb24_to_rgb24()
 */
void mpix_line_rgb24_to_y8_bt709(const uint8_t *src, uint8_t *dst, uint16_t width);

#endif /** @} */
