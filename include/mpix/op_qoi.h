/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_op_palettize mpix/op_palettize.h
 * @brief Implementing new palettization operations
 * @{
 */
#ifndef MPIX_QOI_H
#define MPIX_QOI_H

#include <stdlib.h>
#include <stdint.h>

#include <mpix/op.h>

/** @internal */
struct mpix_qoi_palette_op {
	/** Fields common to all operations. */
	struct mpix_base_op base;
	/** Color palette used for the conversion */
	struct mpix_palette *palette;
	/** Table of alpha values to compensate the hash function */
	uint8_t alpha[1u << 8];
};

/** @internal */
struct mpix_qoi_convert_op {
	/** Fields common to all operations. */
	struct mpix_base_op base;
	/** Array of previously seen pixels */
	uint8_t qoi_cache[64 * 3];
	/** The last seend pixel value just before the new pixel to encode */
	uint8_t qoi_prev[3];
	/** Size of the ongoing run */
	uint8_t qoi_run_length;
};

/**
 * @brief Define a new palettization operation: from a pixel format to indexed colors.
 *
 * @param id Short identifier to differentiate operations of the same category.
 * @param fn Function converting one input line.
 * @param format_in The input format for that operation.
 * @param format_out The Output format for that operation.
 */
#define MPIX_REGISTER_QOI_PALETTE_OP(id, op, fmt_src, fmt_dst)                                     \
	const struct mpix_qoi_palette_op mpix_qoi_palette_op_##id = {                              \
		.base.name = ("qoi_palette_" #id),                                                 \
		.base.format_src = (MPIX_FMT_##fmt_src),                                           \
		.base.format_dst = (MPIX_FMT_##fmt_dst),                                           \
		.base.window_size = 1,                                                             \
		.base.run = op,                                                                    \
	}

/**
 * @brief Define a new palettization operation: from a pixel format to indexed colors.
 *
 * @param id Short identifier to differentiate operations of the same category.
 * @param fn Function converting one input line.
 * @param format_in The input format for that operation.
 * @param format_out The Output format for that operation.
 */
#define MPIX_REGISTER_QOI_CONVERT_OP(id, op, fmt_src, fmt_dst)                                     \
	const struct mpix_qoi_convert_op mpix_qoi_convert_op_##id = {                              \
		.base.name = ("qoi_convert_" #id),                                                 \
		.base.format_src = (MPIX_FMT_##fmt_src),                                           \
		.base.format_dst = (MPIX_FMT_##fmt_dst),                                           \
		.base.window_size = 1,                                                             \
		.base.run = (op),                                                                  \
	}

/**
 * @brief Encode an image to the QOI compressed image format
 *
 * @param image Image to convert to QOI format.
 * @param max_sz Maximum size of the intermediate buffer to use.
 */
int mpix_image_qoi_encode(struct mpix_image *img, size_t max_sz);

#endif /** @} */
