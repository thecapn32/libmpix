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
	stats->histogram[mpix_rgb24_get_luma_bt709(rgb) >> 2]++;

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
}

void mpix_image_stats(struct mpix_image *img, struct mpix_stats *stats)
{
	mpix_stats_from_buf(stats, img->buffer, img->width, img->height, img->format);
}

void mpix_stats_print(struct mpix_stats *stats)
{
	uint8_t rgb[3];

	mpix_port_printf("Y Histogram:\n");
	mpix_print_y_hist(stats->histogram, ARRAY_SIZE(stats->histogram), 10);

	mpix_port_printf("Channel Average:\n");
	rgb[0] = stats->sum_r / stats->nvals;
	rgb[1] = stats->sum_g / stats->nvals;
	rgb[2] = stats->sum_b / stats->nvals;
	mpix_port_printf(" #%02x%02x%02x ", rgb[0], rgb[1], rgb[2]);
	mpix_print_truecolor(rgb, rgb);
	mpix_print_truecolor(rgb, rgb);
	mpix_port_printf("\x1b[m"); /* Reset to normal color */
	mpix_port_printf(" for %u values sampled\n", stats->nvals);
}
