/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <mpix/genlist.h>
#include <mpix/image.h>
#include <mpix/op_resize.h>

static const struct mpix_op **mpix_resize_op_list;

int mpix_image_resize(struct mpix_image *img, uint16_t width, uint16_t height)
{
	const struct mpix_op *op = NULL;
	int ret;

	for (size_t i = 0; i < ARRAY_SIZE(mpix_resize_op_list); i++) {
		const struct mpix_op *tmp = mpix_resize_op_list[i];

		if (tmp->format_in == img->format) {
			op = tmp;
			break;
		}
	}

	if (op == NULL) {
		MPIX_ERR("Resize operation for %s not found", MPIX_FOURCC_TO_STR(img->format));
		return mpix_image_error(img, -ENOSYS);
	}

	ret = mpix_image_append_uncompressed(img, op);
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

__attribute__((weak))
void mpix_resize_frame_raw24(const uint8_t *src_buf, size_t src_width, size_t src_height,
				    uint8_t *dst_buf, size_t dst_width, size_t dst_height)
{
	mpix_resize_frame(src_buf, src_width, src_height, dst_buf, dst_width, dst_height, 24);
}

__attribute__((weak))
void mpix_resize_frame_raw16(const uint8_t *src_buf, size_t src_width, size_t src_height,
				    uint8_t *dst_buf, size_t dst_width, size_t dst_height)
{
	mpix_resize_frame(src_buf, src_width, src_height, dst_buf, dst_width, dst_height, 16);
}

__attribute__((weak))
void mpix_resize_frame_raw8(const uint8_t *src_buf, size_t src_width, size_t src_height,
				   uint8_t *dst_buf, size_t dst_width, size_t dst_height)
{
	mpix_resize_frame(src_buf, src_width, src_height, dst_buf, dst_width, dst_height, 8);
}

static inline void mpix_resize_op(struct mpix_op *op, uint8_t bits_per_pixel)
{
	struct mpix_op *next = op->next;
	uint16_t prev_offset = (op->line_offset + 1) * next->height / op->height;
	const uint8_t *line_in = mpix_op_get_input_line(op);
	uint16_t next_offset = (op->line_offset + 1) * next->height / op->height;

	for (uint16_t i = 0; prev_offset + i < next_offset; i++) {
		mpix_resize_line(line_in, op->width, mpix_op_get_output_line(op), next->width,
				 bits_per_pixel);
		mpix_op_done(op);
	}
}

__attribute__((weak))
void mpix_resize_op_raw24(struct mpix_op *op)
{
	mpix_resize_op(op, 24);
}
MPIX_REGISTER_RESIZE_OP(rgb24, mpix_resize_op_raw24, RGB24);
MPIX_REGISTER_RESIZE_OP(yuv24, mpix_resize_op_raw24, YUV24);

__attribute__((weak))
void mpix_resize_op_raw16(struct mpix_op *op)
{
	mpix_resize_op(op, 16);
}
MPIX_REGISTER_RESIZE_OP(rgb565, mpix_resize_op_raw16, RGB565);
MPIX_REGISTER_RESIZE_OP(rgb565x, mpix_resize_op_raw16, RGB565X);

__attribute__((weak))
void mpix_resize_op_raw8(struct mpix_op *op)
{
	mpix_resize_op(op, 8);
}
MPIX_REGISTER_RESIZE_OP(grey, mpix_resize_op_raw8, GREY);
MPIX_REGISTER_RESIZE_OP(rgb332, mpix_resize_op_raw8, RGB332);

static const struct mpix_op **mpix_resize_op_list = (const struct mpix_op *[]){
	MPIX_LIST_RESIZE_OP
};
