/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_zephyr_h mpix/zephyr.h
 * @brief Zephyr-specific functions
 * @{
 */
#ifndef MPIX_ZEPHYR_H
#define MPIX_ZEPHYR_H

#include <zephyr/drivers/video.h>
#include <mpix/image.h>
#include <mpix/types.h>

/**
 * @brief Initialize an image from a video buffer.
 *
 * @param img Image to initialize.
 * @param vbuf Video buffer that contains the image data to process.
 * @param fmt Video format describing the buffer.
 */
static inline void mpix_image_from_vbuf(struct mpix_image *img, struct video_buffer *vbuf,
					struct video_format *vfmt)
{
	struct mpix_format fmt =
		{ .width = vfmt->width, .height = vfmt->height, .fourcc = vfmt->pixelformat };

	/* Zephyr 3.6 compat */
	if (fmt.fourcc == MPIX_FOURCC('B', 'G', 'G', 'R')) {
		fmt.fourcc = MPIX_FMT_SBGGR8;
	}

	mpix_image_from_buf(img, vbuf->buffer, vbuf->bytesused, &fmt);
}

/**
 * @brief Initialize an image from a memory buffer.
 *
 * @param img Image being processed.
 * @param vbuf Video buffer that receives the image data.
 * @return 0 on success or negative error code.
 */
static inline int mpix_image_to_vbuf(struct mpix_image *img, struct video_buffer *vbuf)
{
	int err = mpix_image_to_buf(img, vbuf->buffer, vbuf->size);
	vbuf->bytesused = mpix_ring_used_size(&img->last_op->ring);
	return err;
}

#endif /** @} */
