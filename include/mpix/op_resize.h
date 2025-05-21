/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_op_resize mpix/op_resize.h
 * @brief Implementing resizing operations
 * @{
 */
#ifndef MPIX_OP_RESIZE_H
#define MPIX_OP_RESIZE_H

#include <stdint.h>

#include <mpix/op.h>

/**
 * @brief Define a format conversion operation.
 *
 * Invoking this macro suffices for @ref mpix_image_convert() to include the extra format.
 *
 * @param id Short identifier to differentiate operations of the same type.
 * @param fn Function converting one input line.
 * @param fmt The pixel format of the data resized.
 */
#define MPIX_REGISTER_RESIZE_OP(id, fn, fmt)                                                       \
	const struct mpix_op mpix_resize_op_##id = {                                               \
		.name = ("resize_" #id),                                                           \
		.format_in = (MPIX_FMT_##fmt),                                                     \
		.format_out = (MPIX_FMT_##fmt),                                                    \
		.window_size = 1,                                                                  \
		.run = (fn),                                                                       \
	}

/**
 * @brief Resize an 24-bit per pixel frame by subsampling the pixels horizontally/vertically.
 *
 * @param src_buf Input buffer to resize
 * @param src_width Width of the input in number of pixels.
 * @param src_height Height of the input in number of pixels.
 * @param dst_buf Output buffer in which the stretched/compressed is stored.
 * @param dst_width Width of the output in number of pixels.
 * @param dst_height Height of the output in number of pixels.
 */
void mpix_resize_frame_raw24(const uint8_t *src_buf, size_t src_width, size_t src_height,
			     uint8_t *dst_buf, size_t dst_width, size_t dst_height);
/**
 * @brief Resize an 16-bit per pixel frame by subsampling the pixels horizontally/vertically.
 * @copydetails mpix_resize_frame_raw24()
 */
void mpix_resize_frame_raw16(const uint8_t *src_buf, size_t src_width, size_t src_height,
			     uint8_t *dst_buf, size_t dst_width, size_t dst_height);
/**
 * @brief Resize an 8-bit per pixel frame by subsampling the pixels horizontally/vertically.
 * @copydetails mpix_resize_frame_raw24()
 */
void mpix_resize_frame_raw8(const uint8_t *src_buf, size_t src_width, size_t src_height,
			    uint8_t *dst_buf, size_t dst_width, size_t dst_height);

#endif /** @} */
