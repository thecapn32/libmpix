/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_ipa mpix/ipa.h
 * @brief Image Processing Algorithms (auto-exposure, etc.) [EXPERIMENTAL]
 * @{
 */
#ifndef MPIX_IPA_H
#define MPIX_IPA_H

#include <stdint.h>
#include <stddef.h>

#include <mpix/stats.h>
#include <mpix/op_isp.h>

struct mpix_ipa {
	/** Pointer to user-provided data that represents a video device */
	void *dev;
	/** Current sensor exposure value */
	int32_t exposure_level;
	/** Maximum sensor exposure value */
	int32_t exposure_max;
	/** The ISP controls */
	struct mpix_isp isp;
};

/**
 * @brief Configure the video device to use for sending controls such as exposure.
 *
 * @param ipa The IPA context to initialize.
 * @param dev The device pointer, passed to functions such as @ref mpix_port_set_exposure().
 */
int mpix_ipa_init(struct mpix_ipa *ipa, void *dev);

/**
 * @brief Apply the current exposure level to the targetted video device.
 * @param ipa The collection of all controls to propagate to the source device.
 */
int mpix_ipa_update_controls(struct mpix_ipa *ipa);

/**
 * @brief Run auto-exposure algorithm to update the exposure control value.
 *
 * @param ipa The current Image Processing Algorithm (IPA) context.
 * @param stats The statistics used to control the exposure.
 */
void mpix_ipa_do_aec(struct mpix_ipa *ipa, struct mpix_stats *stats);

/**
 * @brief Run Black Level Correction (BLC) algorithm to update the black level.
 *
 * @param ipa The current Image Processing Algorithm (IPA) context.
 * @param stats The statistics used to control the black level and then updated.
 */
void mpix_ipa_do_blc(struct mpix_ipa *ipa, struct mpix_stats *stats);

/**
 * @brief Run Auto White Balance algorithm to update the color balance.
 *
 * @param ipa The current Image Processing Algorithm (IPA) context.
 * @param stats The statistics used to control the white balance and then updated.
 */
void mpix_ipa_do_awb(struct mpix_ipa *ipa, struct mpix_stats *stats);

#endif /** @} */
