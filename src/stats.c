/* SPDX-License-Identifier: Apache-2.0 */

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <mpix/image.h>
#include <mpix/stats.h>
#include <mpix/sample.h>
#include <mpix/print.h>
#include <mpix/op_convert.h>

/* Arbitrary value estimated good enough for most cases */
#define MPIX_STATS_DEFAULT_NVALS 1000

static inline void mpix_stats_add_sample(struct mpix_stats *stats, uint8_t rgb[3])
{
	/* Histogram statistics: use BT.601 like libcamera, reduce precision to fit hist[64] */
	stats->y_histogram[mpix_rgb24_get_luma_bt709(rgb) >> 2]++;

	/* RGB statistics */
	stats->sum_r += rgb[0];
	stats->sum_g += rgb[1];
	stats->sum_b += rgb[2];
}

void mpix_stats_from_buf(struct mpix_stats *stats,
			 const uint8_t *buf, uint16_t width, uint16_t height, uint32_t fourcc)
{
	uint8_t nvals = stats->nvals > 0 ? stats->nvals : MPIX_STATS_DEFAULT_NVALS;

	memset(stats, 0x00, sizeof(*stats));

	stats->nvals = nvals;

	for (uint16_t i = 0; i < nvals; i++) {
		uint8_t rgb[3];

		mpix_sample_random_rgb(buf, width, height, fourcc, rgb);
		mpix_stats_add_sample(stats, rgb);
	}

	for (size_t i = 0; i < ARRAY_SIZE(stats->y_histogram_vals); i++) {
		const uint8_t step = 256 / ARRAY_SIZE(stats->y_histogram_vals);

		/* Initialize to the middle value of each range */
		stats->y_histogram_vals[i] = i * step + step / 2;
	}
}

void mpix_image_stats(struct mpix_image *img, struct mpix_stats *stats)
{
	mpix_stats_from_buf(stats, img->buffer, img->width, img->height, img->format);
}

void mpix_stats_print(struct mpix_stats *stats)
{
	uint8_t rgb[3] = {
		stats->sum_r / stats->nvals,
		stats->sum_g / stats->nvals,
		stats->sum_b / stats->nvals,
	};

	mpix_port_printf("Average #%02x%02x%02x ", rgb[0], rgb[1], rgb[2]);
	mpix_print_truecolor(rgb, rgb);
	mpix_port_printf(" \x1b[m for %u values sampled\n", stats->nvals);
	mpix_print_y_hist(stats->y_histogram, ARRAY_SIZE(stats->y_histogram), 10);
}

uint8_t mpix_stats_get_y_mean(struct mpix_stats *stats)
{
	uint32_t nvals = 0;

	for (int i = 0; i < ARRAY_SIZE(stats->y_histogram); i++) {
		nvals += stats->y_histogram[i];

		if (nvals >= stats->nvals / 2) {
			return i * 256 / ARRAY_SIZE(stats->y_histogram);
		}
	}

	assert(0 /* The histogram is expected to contain all values */);
	return 0;
}
