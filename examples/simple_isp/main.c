/* SPDX-License-Identifier: Apache-2.0 */

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <mpix/formats.h>
#include <mpix/image.h>
#include <mpix/operation.h>
#include <mpix/print.h>
#include <mpix/ring.h>
#include <mpix/utils.h>

const struct mpix_format src_fmt = { .width = 640, .height = 480, .fourcc = MPIX_FMT_RGB24 };
const struct mpix_format dst_fmt = { .width = 640, .height = 480, .fourcc = MPIX_FMT_RGB24 };

#define CHECK(x) \
	({ int err = (x); if (err) { printf("error: %s: %s\n", #x, strerror(-err)); exit(1); } })

int simple_isp_demo(uint8_t *src_buf, size_t src_size, const struct mpix_format *src_fmt,
		    uint8_t *dst_buf, size_t dst_size)
{
	struct mpix_image img = {};
	int32_t color_matrix_q10[9] = {
		1.00 * (1 << 10), 0.00 * (1 << 10), 0.00 * (1 << 10),
		0.00 * (1 << 10), 1.00 * (1 << 10), 0.00 * (1 << 10),
		0.00 * (1 << 10), 0.00 * (1 << 10), 1.00 * (1 << 10),
	};

	mpix_image_from_buf(&img, src_buf, src_size, src_fmt);

	/* Add the operations to the pipeline */
	CHECK(mpix_image_correct_black_level(&img));
	CHECK(mpix_image_correct_gamma(&img));
	CHECK(mpix_image_correct_white_balance(&img));
	CHECK(mpix_image_correct_color_matrix(&img));

	/* Control the pipeline image tuning */
	CHECK(mpix_image_ctrl_value(&img, MPIX_CID_BLACK_LEVEL,  0));
	CHECK(mpix_image_ctrl_value(&img, MPIX_CID_RED_BALANCE,  1.3 * (1 << 10)));
	CHECK(mpix_image_ctrl_value(&img, MPIX_CID_BLUE_BALANCE, 1.7 * (1 << 10)));
	CHECK(mpix_image_ctrl_value(&img, MPIX_CID_GAMMA_LEVEL,  0.7 * (1 << 10)));
	CHECK(mpix_image_ctrl_array(&img, MPIX_CID_COLOR_MATRIX, color_matrix_q10));

	/* Process the image and store it to the destination buffer */
	CHECK(mpix_image_to_buf(&img, dst_buf, dst_size));

	mpix_print_pipeline(img.first_op);

	mpix_image_free(&img);

	return 0;
}

int main(int argc, char **argv)
{
	FILE *src_fp;
	FILE *dst_fp;
	size_t src_size = mpix_format_pitch(&src_fmt) * src_fmt.height;
	size_t dst_size = mpix_format_pitch(&dst_fmt) * dst_fmt.height;
	uint8_t *src_buf;
	uint8_t *dst_buf;
	size_t size;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <input-file> <output-file>\n", argv[0]);
		return 1;
	}

	src_fp = fopen(argv[1], "r");
	if (src_fp == NULL) {
		perror(argv[1]);
		return 1;
	}

	dst_fp = fopen(argv[2], "w");
	if (dst_fp == NULL) {
		perror(argv[2]);
		return 1;
	}

	src_buf = malloc(src_size);
	if (src_buf == NULL) {
		perror("allocating source buffer");
		return 1;
	}

	dst_buf = malloc(dst_size);
	if (dst_buf == NULL) {
		perror("allocating destination buffer");
		return 1;
	}

	size = fread(src_buf, 1, src_size, src_fp);
	if (size != src_size) {
		perror(argv[1]);
		return 1;
	}

	simple_isp_demo(src_buf, src_size, &src_fmt, dst_buf, dst_size);

	size = fwrite(dst_buf, 1, dst_size, dst_fp);
	if (size != dst_size) {
		perror(argv[2]);
		return 1;
	}

	fclose(src_fp);
	fclose(dst_fp);
	free(src_buf);
	free(dst_buf);

	return 0;
}
