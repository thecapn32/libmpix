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

struct mpix_ctrl {
	/** Minimum value a control should set */
	int32_t min;
	/** Maximum value a control should set */
	int32_t max;
	/** Current value that needs to be synchronized to the sensor */
	int32_t val;
};

struct mpix_ipa {
	/** Pointer to user-provided data that represents a video device */
	void *dev;
	/** Exposure level to be sent to the image sensor */
	struct mpix_ctrl exposure;
	/** Black level value that is to be removed to every pixel */
	struct mpix_ctrl black_level;
	/** Gamma level to be applied to the pixels */
	struct mpix_ctrl gamma;
};

/**
 * @brief Run auto-exposure algorithm to update the exposure control value.
 */
void mpix_ipa_do_aec(struct mpix_ipa *ipa, struct mpix_stats *stats);

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

#endif /** @} */
