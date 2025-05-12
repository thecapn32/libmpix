/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_kernel mpix/kernel.h
 * @brief Implementing kernel operations
 * @{
 */
#ifndef MPIX_KERNEL_H
#define MPIX_KERNEL_H

#include <stdint.h>
#include <stdlib.h>

#include <mpix/operation.h>

/**
 * @brief Define a new 5x5 kernel conversion operation.
 *
 * @param fn Function converting 5 input lines into 1 output line.
 * @param t Kernel operation type
 * @param fmt The input format for that operation.
 */
#define MPIX_DEFINE_KERNEL_5X5_OPERATION(fn, t, fmt)                                              \
	static const STRUCT_SECTION_ITERABLE_ALTERNATE(mpix_kernel, mpix_operation,              \
						       fn##_op) = {                                \
		.name = #fn,                                                                       \
		.format_in = (MPIX_FORMAT_##fmt),                                                 \
		.format_out = (MPIX_FORMAT_##fmt),                                                \
		.window_size = 5,                                                                  \
		.run = mpix_kernel_5x5_op,                                                        \
		.arg0 = fn,                                                                        \
		.type = t,                                                                         \
	}

/**
 * @brief Define a new 3x3 kernel conversion operation.
 *
 * @param fn Function converting 3 input lines into 1 output line.
 * @param t Kernel operation type
 * @param fmt The input format for that operation.
 */
#define MPIX_DEFINE_KERNEL_3X3_OPERATION(fn, t, fmt)                                              \
	static const STRUCT_SECTION_ITERABLE_ALTERNATE(mpix_kernel, mpix_operation,              \
						       fn##_op) = {                                \
		.name = #fn,                                                                       \
		.format_in = (MPIX_FORMAT_##fmt),                                                 \
		.format_out = (MPIX_FORMAT_##fmt),                                                \
		.window_size = 3,                                                                  \
		.run = mpix_kernel_3x3_op,                                                        \
		.arg0 = fn,                                                                         \
		.type = t,                                                                         \
	}

/**
 * @brief Helper to turn a 5x5 kernel conversion function into an operation.
 *
 * The line conversion function is free to perform any processing on the input lines and expected
 * to produce one output line.
 *
 * The line conversion function is to be provided in @c op->arg0.
 *
 * @param op Current operation in progress.
 */
void mpix_kernel_5x5_op(struct mpix_operation *op);

/**
 * @brief Helper to turn a 3x3 kernel conversion function into an operation.
 * @copydetails mpix_kernel_5x5_op()
 */
void mpix_kernel_3x3_op(struct mpix_operation *op);

/**
 * Available kernel operations to apply to the image.
 */
enum mpix_kernel_type {
	/** Identity kernel: no change, the input is the same as the output */
	MPIX_KERNEL_IDENTITY,
	/** Edge detection kernel: keep only an outline of the edges */
	MPIX_KERNEL_EDGE_DETECT,
	/** Gaussian blur kernel: apply a blur onto an image following a Gaussian curve */
	MPIX_KERNEL_GAUSSIAN_BLUR,
	/** Sharpen kernel: accentuate the edges, making the image look less blurry */
	MPIX_KERNEL_SHARPEN,
	/** Denoise kernel: remove the parasitic image noise using the local median value */
	MPIX_KERNEL_DENOISE,
};

/**
 * @brief Apply a 3x3 identity kernel to an RGB24 input window and produce one RGB24 line.
 *
 * @param in Array of input line buffers to convert.
 * @param out Pointer to the output line converted.
 * @param width Width of the input and output lines in pixels.
 */
void mpix_identity_rgb24_3x3(const uint8_t *in[3], uint8_t *out, uint16_t width);
/**
 * @brief Apply a 5x5 identity kernel to an RGB24 input window and produce one RGB24 line.
 * @copydetails mpix_identity_rgb24_3x3()
 */
void mpix_identity_rgb24_5x5(const uint8_t *in[5], uint8_t *out, uint16_t width);
/**
 * @brief Apply a 3x3 sharpen kernel to an RGB24 input window and produce one RGB24 line.
 * @copydetails mpix_identity_rgb24_3x3()
 */
void mpix_sharpen_rgb24_3x3(const uint8_t *in[3], uint8_t *out, uint16_t width);
/**
 * @brief Apply a 5x5 unsharp kernel to an RGB24 input window and produce one RGB24 line.
 * @copydetails mpix_identity_rgb24_3x3()
 */
void mpix_sharpen_rgb24_5x5(const uint8_t *in[5], uint8_t *out, uint16_t width);
/**
 * @brief Apply a 3x3 edge detection kernel to an RGB24 input window and produce one RGB24 line.
 * @copydetails mpix_identity_rgb24_3x3()
 */
void mpix_edgedetect_rgb24_3x3(const uint8_t *in[3], uint8_t *out, uint16_t width);
/**
 * @brief Apply a 3x3 gaussian blur kernel to an RGB24 input window and produce one RGB24 line.
 * @copydetails mpix_identity_rgb24_3x3()
 */
void mpix_gaussianblur_rgb24_3x3(const uint8_t *in[3], uint8_t *out, uint16_t width);
/**
 * @brief Apply a 3x3 median denoise kernel to an RGB24 input window and produce one RGB24 line.
 * @copydetails mpix_identity_rgb24_3x3()
 */
void mpix_median_rgb24_3x3(const uint8_t *in[3], uint8_t *out, uint16_t width);
/**
 * @brief Apply a 5x5 median denoise kernel to an RGB24 input window and produce one RGB24 line.
 * @copydetails mpix_identity_rgb24_3x3()
 */
void mpix_median_rgb24_5x5(const uint8_t *in[5], uint8_t *out, uint16_t width);

#endif /** @} */
