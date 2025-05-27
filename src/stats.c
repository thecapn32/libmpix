/* SPDX-License-Identifier: Apache-2.0 */

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <mpix/op_convert.h>
#include <mpix/sample.h>
#include <mpix/stats.h>

/* Channel average statistics */

void mpix_stats_rgb_avg(const uint8_t *buf, uint16_t width, uint16_t height, uint32_t fourcc,
			uint8_t avg[3], uint16_t nval)
{
	uint32_t sums[3] = {0, 0, 0};

	for (uint16_t n = 0; n < nval; n++) {
		uint8_t rgb[3];

		mpix_sample_random_rgb(buf, width, height, fourcc, rgb);

		sums[0] += rgb[0];
		sums[1] += rgb[1];
		sums[2] += rgb[2];
	}

	avg[0] = sums[0] / nval;
	avg[1] = sums[1] / nval;
	avg[2] = sums[2] / nval;
}

/* RGB24 histogram statistics */

void mpix_stats_rgb_hist(const uint8_t *buf, uint16_t width, uint16_t height, uint32_t fourcc,
			 uint16_t *hist, size_t hist_size, uint16_t nval)
{
	uint8_t bit_depth = LOG2(hist_size / 3);

	assert(hist_size % 3 == 0 /* Each of R, G, B channel should have the same size */);
	assert(1 << bit_depth == hist_size / 3 /* Each channel size should be a power of two */);

	memset(hist, 0x00, hist_size * sizeof(*hist));

	for (uint16_t n = 0; n < nval; n++) {
		uint8_t rgb[3];

		mpix_sample_random_rgb(buf, width, height, fourcc, rgb);

		hist[(rgb[0] >> (BITS_PER_BYTE - bit_depth)) * 3 + 0]++;
		hist[(rgb[1] >> (BITS_PER_BYTE - bit_depth)) * 3 + 1]++;
		hist[(rgb[2] >> (BITS_PER_BYTE - bit_depth)) * 3 + 2]++;
	}
}

/* Y8 histogram statistics
 * Use BT.709 (sRGB) as an arbitrary choice, instead of BT.601 like libcamera
 */

void mpix_stats_luma_hist(const uint8_t *buf, uint16_t width, uint16_t height, uint32_t fourcc,
			  uint16_t *hist, size_t hist_size, uint16_t nval)
{
	uint8_t bit_depth = LOG2(hist_size);

	assert(1 << bit_depth == hist_size /* Size should be a power of two */);

	memset(hist, 0x00, hist_size * sizeof(*hist));

	for (uint16_t n = 0; n < nval; n++) {
		uint8_t rgb[3];
		uint8_t luma;

		mpix_sample_random_rgb(buf, width, height, fourcc, rgb);
		luma = mpix_rgb24_get_luma_bt709(rgb);

		hist[luma >> (BITS_PER_BYTE - bit_depth)]++;
	}
}
