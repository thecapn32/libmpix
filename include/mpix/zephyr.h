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
 * @brief Initialize an image with Zephyr native types.
 *
 * @param img Image to initialize.
 * @param vbuf Video buffer that contains the image data to process.
 * @param fmt Video format describing the buffer.
 */
static inline void mpix_zephyr_set_format(struct mpix_image *img, struct video_format *vfmt)
{
	img->fmt.width = vfmt->width;
	img->fmt.height = vfmt->height;

	img->fmt.fourcc =
		/* Zephyr 3.6 compat */
		(vfmt->pixelformat == MPIX_FOURCC('B', 'G', 'G', 'R')) ? MPIX_FMT_SBGGR8 :
		(vfmt->pixelformat == MPIX_FOURCC('R', 'G', 'G', 'B')) ? MPIX_FMT_SRGGB8 :
		(vfmt->pixelformat == MPIX_FOURCC('G', 'R', 'B', 'G')) ? MPIX_FMT_SGRBG8 :
		(vfmt->pixelformat == MPIX_FOURCC('G', 'B', 'R', 'G')) ? MPIX_FMT_SGBRG8 :
		/* Fourcc are standards so direct match is possible */
		vfmt->pixelformat;
}

/**
 * @brief Initialize an image from a zephyr video buffer.
 *
 * @param img Image to initialize.
 * @param vbuf Video buffer that contains the image data to process.
 * @param fmt Video format describing the buffer.
 */
static inline void mpix_image_from_vbuf(struct mpix_image *img, struct video_buffer *vbuf,
					struct video_format *vfmt)
{
	mpix_zephyr_set_format(img, vfmt);
	mpix_image_from_buf(img, vbuf->buffer, vbuf->bytesused, &img->fmt);
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
