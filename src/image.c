/*
 * Copyright (c) 2025 tinyVision.ai Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#include <mpix/image.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(mpix_image, CONFIG_MPIX_LOG_LEVEL);

K_HEAP_DEFINE(mpix_heap, CONFIG_MPIX_HEAP_SIZE);

static void mpix_image_free(struct mpix_image *img)
{
	struct mpix_operation *op, *tmp;

	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&img->operations, op, tmp, node) {
		if (op->is_heap) {
			k_heap_free(&mpix_heap, op->ring.buffer);
		}
		memset(op, 0x00, sizeof(*op));
		k_heap_free(&mpix_heap, op);
	}
	memset(&img->operations, 0x00, sizeof(img->operations));
}

int mpix_image_error(struct mpix_image *img, int err)
{
	if (err != 0 && img->err == 0) {
		mpix_image_free(img);
		img->err = err;
	}
	return err;
}

static void *mpix_image_alloc(struct mpix_image *img, size_t size)
{
	void *mem;

	mem = k_heap_alloc(&mpix_heap, size, K_NO_WAIT);
	if (mem == NULL) {
		LOG_ERR("Out of memory whle allocating %zu bytes", size);
	}
	return mem;
}

int mpix_image_add_operation(struct mpix_image *img, const struct mpix_operation *template,
			      size_t buffer_size, size_t threshold)
{
	struct mpix_operation *op;

	if (img->err) {
		return -ECANCELED;
	}

	if (template->format_in != img->format) {
		LOG_ERR("Wrong format for this operation: image has %s, operation uses %s",
			MPIX_FORMAT_TO_STR(template->format_in), MPIX_FORMAT_TO_STR(img->format));
		return mpix_image_error(img, -EINVAL);
	}

	op = mpix_image_alloc(img, sizeof(*op));
	if (op == NULL) {
		return mpix_image_error(img, -ENOMEM);
	}

	memcpy(op, template, sizeof(*op));
	op->threshold = threshold;
	op->width = img->width;
	op->height = img->height;
	op->ring.buffer = NULL; /* allocated later */
	op->ring.size = buffer_size;

	img->format = op->format_out;

	sys_slist_append(&img->operations, &op->node);

	return 0;
}

int mpix_image_add_uncompressed(struct mpix_image *img, const struct mpix_operation *template)
{
	size_t pitch = img->width * mpix_bits_per_pixel(img->format) / BITS_PER_BYTE;
	size_t size = template->window_size * pitch;

	return mpix_image_add_operation(img, template, size, size);
}

int mpix_image_process(struct mpix_image *img)
{
	struct mpix_operation *op;
	uint8_t *p;

	if (img->err) {
		return -ECANCELED;
	}

	if (img->buffer == NULL) {
		LOG_ERR("No input buffer configured");
		return mpix_image_error(img, -ENOBUFS);
	}

	op = SYS_SLIST_PEEK_HEAD_CONTAINER(&img->operations, op, node);
	if (op == NULL) {
		LOG_ERR("No operation to perform on image");
		return mpix_image_error(img, -ENOSYS);
	}

	if (ring_buf_capacity_get(&op->ring) < op->ring.size) {
		LOG_ERR("Not enough space (%u) in input buffer to run the first operation (%u)",
			ring_buf_capacity_get(&op->ring), op->ring.size);
		return mpix_image_error(img, -ENOSPC);
	}

	ring_buf_init(&op->ring, img->size, img->buffer);
	ring_buf_put_claim(&op->ring, &p, img->size);
	ring_buf_put_finish(&op->ring, img->size);

	while ((op = SYS_SLIST_PEEK_NEXT_CONTAINER(op, node)) != NULL) {
		if (op->ring.buffer == NULL) {
			op->ring.buffer = mpix_image_alloc(img, op->ring.size);
			if (op->ring.buffer == NULL) {
				return mpix_image_error(img, -ENOMEM);
			}
			op->is_heap = true;
		}
	}

	SYS_SLIST_FOR_EACH_CONTAINER(&img->operations, op, node) {
		LOG_DBG("- %s %ux%u to %s, %s, threshold %u",
			MPIX_FORMAT_TO_STR(op->format_in), op->width, op->height,
			MPIX_FORMAT_TO_STR(op->format_out), op->name, op->threshold);
	}

	op = SYS_SLIST_PEEK_HEAD_CONTAINER(&img->operations, op, node);

	mpix_operation_run(op);
	mpix_image_free(img);

	return 0;
}

void mpix_image_from_buffer(struct mpix_image *img, uint8_t *buffer, size_t size,
			     uint16_t width, uint16_t height, uint32_t format)
{
	memset(img, 0x00, sizeof(*img));
	img->buffer = buffer;
	img->size = size;
	img->width = width;
	img->height = height;
	img->format = format;
}

void mpix_image_from_vbuf(struct mpix_image *img, struct video_buffer *vbuf,
			   struct video_format *fmt)
{
	mpix_image_from_buffer(img, vbuf->buffer, vbuf->size, fmt->width, fmt->height,
				fmt->pixelformat);
}

int mpix_image_to_buffer(struct mpix_image *img, uint8_t *buffer, size_t size)
{
	struct mpix_operation *op;
	int ret;

	if (img->err) {
		return -ECANCELED;
	}

	op = mpix_image_alloc(img, sizeof(struct mpix_operation));
	if (op == NULL) {
		return mpix_image_error(img, -ENOMEM);
	}

	memset(op, 0x00, sizeof(*op));
	op->name = __func__;
	op->threshold = size;
	op->format_in = img->format;
	op->format_out = img->format;
	op->width = img->width;
	op->height = img->height;
	op->ring.buffer = buffer;
	op->ring.size = size;

	sys_slist_append(&img->operations, &op->node);

	ret = mpix_image_process(img);

	img->buffer = buffer;
	img->size = size;

	return ret;
}

int mpix_image_to_vbuf(struct mpix_image *img, struct video_buffer *vbuf)
{
	int ret;

	ret = mpix_image_to_buffer(img, vbuf->buffer, vbuf->size);
	vbuf->bytesused = img->size;

	return ret;
}
