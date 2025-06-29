/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_op_jpeg mpix/op_jpeg.h
 * @brief Low-level JPEG encoding operations
 * @{
 */
#ifndef MPIX_JPEG_H
#define MPIX_JPEG_H

#include <stdlib.h>
#include <stdint.h>

#include <mpix/op.h>

/** @internal */
struct mpix_jpeg_op {
	/** Fields common to all operations. */
	struct mpix_base_op base;
};

/**
 * @brief Define a new encoding operation: from a pixel format to the JPEG format.
 *
 * @param id Short identifier to differentiate operations of the same category.
 * @param fn Function converting one input line.
 * @param format_in The input format for that operation.
 * @param format_out The Output format for that operation.
 */
#define MPIX_REGISTER_JPEG_OP(id, op, fmt_src, fmt_dst)                                            \
	const struct mpix_jpeg_op mpix_jpeg_op_##id = {                                            \
		.base.name = ("jpeg_" #id),                                                        \
		.base.fourcc_src = (MPIX_FMT_##fmt_src),                                           \
		.base.fourcc_dst = (MPIX_FMT_##fmt_dst),                                           \
		.base.window_size = 1,                                                             \
		.base.run = (op),                                                                  \
	}

#endif /** @} */
