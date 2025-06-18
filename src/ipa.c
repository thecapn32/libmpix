/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>

#include <mpix/ipa.h>
#include <mpix/stats.h>
#include <mpix/utils.h>
#include <mpix/port.h>
#include <mpix/op_isp.h>

int mpix_ipa_init(struct mpix_ipa *ipa, void *dev)
{
	int ret;

	ipa->dev = dev;

	ret = mpix_port_init_exposure(dev, &ipa->exposure_level, &ipa->exposure_max);
	if (ret < 0) {
		MPIX_ERR("Failed to initialize exposure control");
		return ret;
	}

	return 0;
}

int mpix_ipa_update_controls(struct mpix_ipa *ipa)
{
	if (ipa->dev == NULL) {
		return -ENODEV;
	}
	return mpix_port_set_exposure(ipa->dev, ipa->exposure_level);
}

#ifndef CONFIG_MPIX_AEC_THRESHOLD
/** @brief Threshold value over which trigger an auto-exposure cycle */
#define CONFIG_MPIX_AEC_THRESHOLD 5
#endif

#ifndef CONFIG_MPIX_AEC_CHANGE_RATE
/** @brief Percentage of change to apply per auto-exposure cycle */
#define CONFIG_MPIX_AEC_CHANGE_RATE 30
#endif

void mpix_ipa_do_aec(struct mpix_ipa *ipa, struct mpix_stats *stats)
{
	uint8_t mean = mpix_stats_get_y_mean(stats);
	int32_t val = ipa->exposure_level;

	if (mean > 128 + CONFIG_MPIX_AEC_THRESHOLD) {
		MPIX_INF("Over-exposed at exposure %u, reducing exposure", ipa->exposure_level);
		val = val * (100 - CONFIG_MPIX_AEC_CHANGE_RATE) / 100 - 1;
	} else if (mean < 128 - CONFIG_MPIX_AEC_THRESHOLD) {
		MPIX_INF("Under-exposed at exposure %u, raising exposure", ipa->exposure_level);
		val = val * (100 + CONFIG_MPIX_AEC_CHANGE_RATE) / 100 + 1;
	}

	/* Update the value itself */
	ipa->exposure_level = CLAMP(val, 1, ipa->exposure_max);

	MPIX_DBG("New exposure value: %u/%u", ipa->exposure_level, ipa->exposure.max);
}

#ifndef CONFIG_MPIX_BLC_THRESHOLD
/** @brief Percentage of pixels to cross before setting the minimum value */
#define CONFIG_MPIX_BLC_THRESHOLD 2
#endif

void mpix_ipa_do_blc(struct mpix_ipa *ipa, struct mpix_stats *stats)
{
	uint16_t sum = 0;

	ipa->isp.black_level = 0;

	/* Seek the first bucket that */
	for (size_t i = 0; i < ARRAY_SIZE(stats->y_histogram); i++) {
		sum += stats->y_histogram[i];

		if (100 * sum / stats->nvals >= CONFIG_MPIX_BLC_THRESHOLD) {
			ipa->isp.black_level = stats->y_histogram_vals[i];
			break;
		}
	}

	/* Update the statistics so that they reflect the change of black level */
	mpix_isp_black_level_raw8(stats->y_histogram_vals, stats->y_histogram_vals,
				  sizeof(stats->y_histogram_vals), &ipa->isp);
	mpix_isp_black_level_rgb24(stats->rgb_average, stats->rgb_average, 1, &ipa->isp);

	MPIX_DBG("New black level: %u", ipa->isp.black_level);
}

/*
 * Simple Gray World strategy for Auto White Balance (AWB).
 *
 * The gain values are shifted from the range [x0, x1] to the range [x1, x2].
 * This means that images are always expected to be more green than blue or red, which is
 * assumed to be the case for all image sensors encountered by this library.
 */
void mpix_ipa_do_awb(struct mpix_ipa *ipa, struct mpix_stats *stats)
{
	int r = MAX(1, stats->rgb_average[0]);
	int g = MAX(1, stats->rgb_average[1]);
	int b = MAX(1, stats->rgb_average[2]);

	ipa->isp.red_level = CLAMP(g / r, 0x00, 0xff);
	ipa->isp.blue_level = CLAMP(g / b, 0x00, 0xff);

	/* Update the statistics so that they reflect the change of white balance */
	mpix_isp_white_balance_rgb24(stats->rgb_average, stats->rgb_average, 1, &ipa->isp);

	MPIX_INF("New red level: %u", ipa->isp.red_level);
	MPIX_INF("New blue level: %u", ipa->isp.blue_level);
}
