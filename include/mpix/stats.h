/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_stats mpix/stats.h
 * @brief Compute statistics on images [EXPERIMENTAL]
 * @{
 */
#ifndef MPIX_STATS_H
#define MPIX_STATS_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Histogram of one or multiple channels.
 */
struct mpix_stats {
	/** Storage for the values. One buffer per channel. Maximum number of channels is 3. */
	uint16_t histogram[64];
	/** Sum of red channel values */
	uint32_t sum_r;
	/** Sum of green channel values */
	uint32_t sum_g;
	/** Sum of blue channel values */
	uint32_t sum_b;
	/** Number of values collected for statistics so far */
	uint16_t nvals;
};

/**
 * @brief Collect red, green, blue channel averages of all pixels in an RGB24 frame.
 *
 * If the @c nvals field of @p stats is non-zero, then this value will be used to select the number
 * of pixels sampled from the image. Otherwise a default will be provided.
 *
 * @param stats Struct collecting the image statistics.
 * @param buf Buffer of pixels in RGB24 format (3 bytes per pixel) to collect the statistics from.
 * @param width Width of the buffer in pixels.
 * @param height Height of the buffer in pixels.
 * @param fourcc Pixel format of the buffer.
 */
void mpix_stats_from_buf(struct mpix_stats *stats,
			 const uint8_t *buf, uint16_t width, uint16_t height, uint32_t fourcc);

#endif /** @} */
