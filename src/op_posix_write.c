/* SPDX-License-Identifier: Apache-2.0 */

#include <unistd.h>

#include <mpix/low_level.h>
#include <mpix/operation.h>

MPIX_REGISTER_OP(posix_write, P_FILE_DESCRIPTOR, P_BUFFER_SIZE);

struct mpix_operation {
	struct mpix_base_op base;
	/* Parameters */
	int file_descriptor;
};

int mpix_add_posix_write(struct mpix_image *img, const int32_t *params)
{
	struct mpix_operation *op;

	/* Parameter validation */
	if (params[P_FILE_DESCRIPTOR] < 0 || params[P_BUFFER_SIZE] < 1) {
		return -EINVAL;
	}

	/* Add an operation */
	op = mpix_op_append(img, MPIX_OP_POSIX_WRITE, sizeof(*op), params[P_BUFFER_SIZE]);
	if (op == NULL) {
		return -ENOMEM;
	}

	/* Store parameters */
	op->file_descriptor = params[P_FILE_DESCRIPTOR];

	return 0;
}

int mpix_run_posix_write(struct mpix_base_op *base)
{
	struct mpix_operation *op = (struct mpix_operation *)base;
	size_t total_size;
	const uint8_t *buf;
	size_t written = 0;

	MPIX_OP_INPUT_PEEK(base, &buf, &total_size);

	while (written < total_size) {
		ssize_t n = write(op->file_descriptor, buf + written, total_size - written);
		if (n < 0) {
			MPIX_ERR("Failed to write %zu bytes to file descriptor %d",
				 total_size - written, op->file_descriptor);
			return -errno;
		}

		written += n;
	}

	MPIX_OP_INPUT_FLUSH(base, total_size);

	return 0;
}
