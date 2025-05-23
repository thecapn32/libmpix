/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_operation mpix/op.h
 * @brief Implementing new types of operations
 * @{
 */
#ifndef MPIX_OPERATION_H
#define MPIX_OPERATION_H

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include <mpix/formats.h>
#include <mpix/port.h>
#include <mpix/ring.h>
#include <mpix/utils.h>

/**
 * @brief One step of a line operation pipeline
 *
 * @c mpix_op structs are chained together into a linked list.
 * Each step of the linked list contain a ring buffer for the input data, and a pointer to a
 * conversion function processing it.
 * Along with extra metadata, this is used to process data as a operation of lines.
 */
struct mpix_op {
	/** Linked-list entry */
	struct mpix_op *next;
	/** Name of the operation, useful for debugging the operation */
	const uint8_t *name;
	/** Operation type in case there are several variants for an operation */
	uint32_t type;
	/** Pixel input format */
	uint32_t format_in;
	/** Pixel output format */
	uint32_t format_out;
	/** Width of the image in pixels */
	uint16_t width;
	/** Height of the image in pixels */
	uint16_t height;
	/** Current position within the frame */
	uint16_t line_offset;
	/** Number of lines of context around the line being converted */
	uint16_t window_size;
	/** Number of bytes of input needed to call @c run() */
	uint16_t threshold;
	/** Whether the ring buffer memory is allocated on the pixel heap or externally */
	bool is_heap;
	/** Ring buffer that keeps track of the position in bytes */
	struct mpix_ring ring;
	/** Function that performs the I/O */
	void (*run)(struct mpix_op *op);
	/** Operation-specific data */
	void *arg0;
	/** Operation-specific data */
	void *arg1;
	/** Timestamp since the op started working in CPU cycles */
	uint32_t start_time_us;
	/** Total time spent working in this op through the operation in CPU cycles */
	uint32_t total_time_us;
};

static inline size_t mpix_op_get_pitch(struct mpix_op *op)
{
	size_t bits_per_pixel;

	bits_per_pixel = mpix_bits_per_pixel(op->format_in);
	assert(bits_per_pixel > 0 /* this pixel format unknown or variable pitch */);

	return op->width * bits_per_pixel / BITS_PER_BYTE;
}

/**
 * @brief Request a pointer the next specified bytes of data from the input buffer
 *
 * This will shift the "read" position by as many bytes.
 * The buffer obtained contains the input data to process.
 * The lines will be considered as converted as soon as @ref mpix_op_done() is called,
 * which will feed the line into the next step of the operation.
 *
 * @param op Current operation in progress.
 * @param size Number of bytes to request.
 * @return The requested buffer, never NULL.
 */
static inline uint8_t *mpix_op_get_input_bytes(struct mpix_op *op, size_t size)
{
	uint8_t *data;

	data = mpix_ring_read(&op->ring, size);
	assert(data != NULL /* Asked for more input bytes than available in the buffer */);

	return data;
}

/**
 * @brief Request a pointer the next specified bytes of data from the output buffer
 *
 * This will shift the "write" position by as many bytes.
 * The buffer obtained can be used to store the processed data.
 * The lines will be considered as converted as soon as @ref mpix_op_done() is called,
 * which will feed the line into the next step of the operation.
 *
 * @param op Current operation in progress.
 * @param size Number of bytes to request.
 * @return The requested buffer, never NULL.
 */
static inline uint8_t *mpix_op_get_output_bytes(struct mpix_op *op, size_t size)
{
	uint8_t *data;

	assert(op->next != NULL /* Missing  operation at the end, cannot get output buffer */);

	MPIX_DBG("'%s' asks for %u output bytes, %u ready",
		op->name, size, mpix_ring_headroom(&op->next->ring));

	data = mpix_ring_write(&op->next->ring, size);
	assert(data != NULL /* Asked for more output bytes than available in the buffer */);

	return data;
}

/**
 * @brief Request a pointer the next specified bytes of data from the input buffer
 *
 * This will not shift the "read" position, but will shift the "peek" position.
 * The buffer obtained contains the input data to process.
 * The lines will be considered as converted as soon as @ref mpix_op_done() is called,
 * which will feed the line into the next step of the operation.
 *
 * @param op Current operation in progress.
 * @param size Number of bytes to request.
 * @return The requested buffer, never NULL.
 */
static inline uint8_t *mpix_op_peek_input_bytes(struct mpix_op *op, size_t size)
{
	uint8_t *data;

	MPIX_DBG("'%s' asks for %u input bytes, %u ready",
		op->name, size, mpix_ring_peekroom(&op->ring));

	data = mpix_ring_peek(&op->ring, size);
	assert(data != NULL /* Asked for more input bytes than peekable in the buffer */);

	return data;
}

static inline uint8_t *mpix_op_get_output_line(struct mpix_op *op)
{
	return mpix_op_get_output_bytes(op, mpix_op_get_pitch(op->next) * 1);
}

/**
 * @brief Get a pointer to a given number of input lines, and consume them from the operation.
 *
 * The lines are considered as processed, which will free them from the input ring buffer, and
 * allow more data to flow in.
 *
 * @param op Current operation in progress.
 * @param nb Number of lines to get in one block.
 * @return Pointer to the requested number of lines, never NULL.
 */
static inline const uint8_t *mpix_op_get_input_lines(struct mpix_op *op, size_t nb)
{
	return mpix_op_get_output_bytes(op, mpix_op_get_pitch(op->next) * nb);
}

/**
 * @brief Shorthand for @ref mpix_op_get_input_lines() to get a single input line.
 *
 * @param op Current operation in progress.
 * @return Pointer to the requested number of lines, never NULL.
 */
static inline const uint8_t *mpix_op_get_input_line(struct mpix_op *op)
{
	return mpix_op_get_input_bytes(op, mpix_op_get_pitch(op) * 1);
}

/**
 * @brief Request a pointer to the next line of data without affecting the input sream.
 *
 * This permits to implement a lookahead operation when one or several lines of context is needed
 * in addition to the line converted.
 *
 * @return The pointer to the input data.
 */
static inline uint8_t *mpix_op_peek_input_lines(struct mpix_op *op, size_t nb)
{
	return mpix_op_peek_input_bytes(op, mpix_op_get_pitch(op->next) * nb);
}

/**
 * @brief Request a pointer to the next line of data without affecting the input sream.
 *
 * This permits to implement a lookahead operation when one or several lines of context is needed
 * in addition to the line converted.
 *
 * @return The pointer to the input data.
 */
static inline uint8_t *mpix_op_peek_input_line(struct mpix_op *op)
{
	return mpix_op_peek_input_bytes(op, mpix_op_get_pitch(op->next) * 1);
}

/**
 * @brief Request a pointer to the entire input buffer content, consumed from the input operation.
 *
 * @param op Current operation in progress.
 * @return The pointer to the input data.
 */
static inline const uint8_t *mpix_op_get_all_input(struct mpix_op *op, size_t *size)
{
	*size = mpix_ring_tailroom(&op->ring);
	return mpix_ring_read(&op->ring, *size);
}

static inline void mpix_op_run(struct mpix_op *op)
{
	if (op != NULL && op->run != NULL) {
		/* Start the counter of the next operation */
		op->start_time_us = mpix_port_get_uptime_us();

		while (mpix_ring_total_used(&op->ring) >= op->threshold &&
		       op->line_offset < op->height) {
			op->run(op);
		}
	}
}

/**
 * @brief Mark the line obtained with @ref mpix_op_get_output_line as converted.
 *
 * This will let the next step of the operation know that a new line was converted.
 * This allows the pipeline to trigger the next step if there is enough data submitted to it.
 *
 * @param op Current operation in progress.
 */
static inline void mpix_op_done(struct mpix_op *op)
{
	uint32_t done_time_us = mpix_port_get_uptime_us();

	/* Flush the timestamp to the counter */
	op->total_time_us += (op->start_time_us == 0) ? (0) : (done_time_us - op->start_time_us);

	/* Run the next operation in the chain now that more data is available */
	mpix_op_run(op->next);

	/* Resuming to this operation, reset the time counter */
	op->start_time_us = mpix_port_get_uptime_us();
}

#endif /** @} */
