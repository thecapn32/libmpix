/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @defgroup mpix_posix_h mpix/posix.h
 * @brief POSIX-specific functions
 * @{
 */
#ifndef MPIX_POSIX_H
#define MPIX_POSIX_H

#include <mpix/types.h>
#include <mpix/pipeline.h>

/**
 * @brief Write an image to an open file
 *
 * Append a "posix_write" operation and write the output to an output file as it is processed.
 *
 * @param img Image to process and write to an output file.
 * @param file_descriptor The POSIX file descriptor number to write to.
 * @param buf_size The size of the output write buffer.
 */
static inline int mpix_image_to_file(struct mpix_image *img, int file_descriptor, size_t buf_size)
{
	int32_t p[] = { (int32_t)file_descriptor, (int32_t)buf_size };
	int err = mpix_pipeline_add(img, MPIX_OP_POSIX_WRITE, p, ARRAY_SIZE(p));
	return err ? err : mpix_pipeline_process(img->first_op, img->buffer, img->size);
}

#endif /** @} */
