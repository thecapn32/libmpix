/*
 * Copyright (c) 2025 tinyVision.ai Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <mpix/resize.h>
#include <mpix/image.h>

LOG_MODULE_REGISTER(mpix_resize, CONFIG_MPIX_LOG_LEVEL);

int mpix_image_resize(struct mpix_image *img, uint16_t width, uint16_t height)
{
	const struct mpix_operation *op = NULL;
	int ret;

	STRUCT_SECTION_FOREACH_ALTERNATE(mpix_resize, mpix_operation, tmp) {
		if (tmp->format_in == img->format) {
			op = tmp;
			break;
		}
	}

	if (op == NULL) {
		LOG_ERR("Resize operation for %s not found", MPIX_FORMAT_TO_STR(img->format));
		return mpix_image_error(img, -ENOSYS);
	}

	ret = mpix_image_add_uncompressed(img, op);
	img->width = width;
	img->height = height;
	return ret;
}

static inline void mpix_resize_line(const uint8_t *src_buf, size_t src_width, uint8_t *dst_buf,
				     size_t dst_width, uint8_t bits_per_pixel)
{
	for (size_t dst_w = 0; dst_w < dst_width; dst_w++) {
		size_t src_w = dst_w * src_width / dst_width;
		size_t src_i = src_w * bits_per_pixel / BITS_PER_BYTE;
		size_t dst_i = dst_w * bits_per_pixel / BITS_PER_BYTE;

		memmove(&dst_buf[dst_i], &src_buf[src_i], bits_per_pixel / BITS_PER_BYTE);
	}
}

static inline void mpix_resize_frame(const uint8_t *src_buf, size_t src_width, size_t src_height,
				      uint8_t *dst_buf, size_t dst_width, size_t dst_height,
				      uint8_t bits_per_pixel)
{
	for (size_t dst_h = 0; dst_h < dst_height; dst_h++) {
		size_t src_h = dst_h * src_height / dst_height;
		size_t src_i = src_h * src_width * bits_per_pixel / BITS_PER_BYTE;
		size_t dst_i = dst_h * dst_width * bits_per_pixel / BITS_PER_BYTE;

		mpix_resize_line(&src_buf[src_i], src_width, &dst_buf[dst_i], dst_width,
				     bits_per_pixel);
	}
}

__weak void mpix_resize_frame_raw24(const uint8_t *src_buf, size_t src_width, size_t src_height,
				    uint8_t *dst_buf, size_t dst_width, size_t dst_height)
{
	mpix_resize_frame(src_buf, src_width, src_height, dst_buf, dst_width, dst_height, 24);
}

__weak void mpix_resize_frame_raw16(const uint8_t *src_buf, size_t src_width, size_t src_height,
				     uint8_t *dst_buf, size_t dst_width, size_t dst_height)
{
	mpix_resize_frame(src_buf, src_width, src_height, dst_buf, dst_width, dst_height, 16);
}

__weak void mpix_resize_frame_raw8(const uint8_t *src_buf, size_t src_width, size_t src_height,
				    uint8_t *dst_buf, size_t dst_width, size_t dst_height)
{
	mpix_resize_frame(src_buf, src_width, src_height, dst_buf, dst_width, dst_height, 8);
}

static inline void mpix_op_resize(struct mpix_operation *op, uint8_t bits_per_pixel)
{
	struct mpix_operation *next = SYS_SLIST_PEEK_NEXT_CONTAINER(op, node);
	uint16_t prev_offset = (op->line_offset + 1) * next->height / op->height;
	const uint8_t *line_in = mpix_operation_get_input_line(op);
	uint16_t next_offset = (op->line_offset + 1) * next->height / op->height;

	for (uint16_t i = 0; prev_offset + i < next_offset; i++) {
		mpix_resize_line(line_in, op->width, mpix_operation_get_output_line(op),
				     next->width, bits_per_pixel);
		mpix_operation_done(op);
	}
}

__weak void mpix_op_resize_raw24(struct mpix_operation *op)
{
	mpix_op_resize(op, 24);
}
MPIX_DEFINE_RESIZE_OPERATION(mpix_op_resize_raw24, RGB24);
MPIX_DEFINE_RESIZE_OPERATION(mpix_op_resize_raw24, YUV24);

__weak void mpix_op_resize_raw16(struct mpix_operation *op)
{
	mpix_op_resize(op, 16);
}
MPIX_DEFINE_RESIZE_OPERATION(mpix_op_resize_raw16, RGB565);
MPIX_DEFINE_RESIZE_OPERATION(mpix_op_resize_raw16, RGB565X);

__weak void mpix_op_resize_raw8(struct mpix_operation *op)
{
	mpix_op_resize(op, 8);
}
MPIX_DEFINE_RESIZE_OPERATION(mpix_op_resize_raw8, GREY);
MPIX_DEFINE_RESIZE_OPERATION(mpix_op_resize_raw8, RGB332);
