/* SPDX-License-Identifier: Apache-2.0 */

#include <mpix/ipa.h>
#include <mpix/stats.h>
#include <mpix/utils.h>
#include <mpix/port.h>

#define CONFIG_MPIX_AEC_THRESHOLD	10
#define CONFIG_MPIX_AEC_CHANGE_PERCENT	30

void mpix_ipa_do_aec(struct mpix_ipa *ipa, struct mpix_stats *stats)
{
	uint8_t mean = mpix_stats_get_y_mean(stats);

	if (mean > 128 + CONFIG_MPIX_AEC_THRESHOLD) {
		MPIX_DBG("Over-exposed at exposure %u, reducing exposure", ipa->exposure.val);
		ipa->exposure.val =
			ipa->exposure.val * (100 - CONFIG_MPIX_AEC_CHANGE_PERCENT) / 100;
		goto clamp;
	}

	if (mean < 128 - CONFIG_MPIX_AEC_THRESHOLD) {
		MPIX_DBG("Under-exposed at exposure %u, raising exposure", ipa->exposure.val);
		ipa->exposure.val =
			ipa->exposure.val * (100 + CONFIG_MPIX_AEC_CHANGE_PERCENT) / 100;
		goto clamp;
	}

clamp:
	ipa->exposure.val = CLAMP(ipa->exposure.val, ipa->exposure.min, ipa->exposure.max);

	/* Avoid get completely stuck with '0 * x = 0' */
	ipa->exposure.val = (ipa->exposure.val < 10) ? (10) : (ipa->exposure.val);

	MPIX_DBG("New exposure value: %u/%u", ipa->exposure.val, ipa->exposure.max);
}

int mpix_ipa_init(struct mpix_ipa *ipa, void *dev)
{
	int ret;

	ipa->dev = dev;

	ret = mpix_port_init_exposure(dev, &ipa->exposure);
	if (ret < 0) {
		MPIX_ERR("Failed to initialize exposure control");
		return ret;
	}

	MPIX_INF("- Exposure control: min %u, max %u", ipa->exposure.min, ipa->exposure.max);

	return 0;
}

int mpix_ipa_update_controls(struct mpix_ipa *ipa)
{
	return mpix_port_set_exposure(ipa->dev, ipa->exposure.val);
}
