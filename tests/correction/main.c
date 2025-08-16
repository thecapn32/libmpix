#include <mpix/op_correction.h>
#include <mpix/test.h>
#include <stdio.h>

#define WIDTH 16
#define HEIGHT 16

uint8_t src[WIDTH * HEIGHT * 3];
uint8_t dst[WIDTH * HEIGHT * 3];

void test_some_matrix(void)
{
	union mpix_correction_any correction = {
		.color_matrix.levels  = {
			/* 1st matrix row */
			2.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 2nd matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			2.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 3rd matrix row */
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
		},
	};

	mpix_correction_color_matrix_rgb24(src, dst, WIDTH * HEIGHT, 0, &correction);

	mpix_test_equal(dst[0],
			(src[0] * correction.color_matrix.levels[0] +
			src[1] * correction.color_matrix.levels[1] +
			src[2] * correction.color_matrix.levels[2]));

	mpix_test_equal(dst[1],
			(src[0] * correction.color_matrix.levels[3] +
			src[1] * correction.color_matrix.levels[4] +
			src[2] * correction.color_matrix.levels[5]));

	mpix_test_equal(dst[2],
			(src[0] * correction.color_matrix.levels[6] +
			src[1] * correction.color_matrix.levels[7] +
			src[2] * correction.color_matrix.levels[8]));
}

void test_identity_matrix(void)
{
	union mpix_correction_any correction = {
		.color_matrix.levels  = {
			/* 1st matrix row */
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 2nd matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 3rd matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
		},
	};

	mpix_correction_color_matrix_rgb24(src, dst, WIDTH * HEIGHT, 0, &correction);

	mpix_test_equal(dst[0], src[0]);
	mpix_test_equal(dst[1], src[1]);
	mpix_test_equal(dst[2], src[2]);

}

void test_red_to_gray_matrix(void)
{
	union mpix_correction_any correction = {
		.color_matrix.levels  = {
			/* 1st matrix row */
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 2nd matrix row */
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 3rd matrix row */
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
		},
	};

	mpix_correction_color_matrix_rgb24(src, dst, WIDTH * HEIGHT, 0, &correction);

	mpix_test_equal(dst[0], src[0]);
	mpix_test_equal(dst[1], 0);
	mpix_test_equal(dst[2], 0);

}

void test_green_to_gray_matrix(void)
{
	union mpix_correction_any correction = {
		.color_matrix.levels  = {
			/* 1st matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 2nd matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 3rd matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
		},
	};

	mpix_correction_color_matrix_rgb24(src, dst, WIDTH * HEIGHT, 0, &correction);

	mpix_test_equal(dst[0], 0);
	mpix_test_equal(dst[1], src[1]);
	mpix_test_equal(dst[2], 0);
}

void test_blue_to_gray_matrix(void)
{
	union mpix_correction_any correction = {
		.color_matrix.levels  = {
			/* 1st matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 2nd matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 3rd matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
		},
	};

	mpix_correction_color_matrix_rgb24(src, dst, WIDTH * HEIGHT, 0, &correction);

	mpix_test_equal(dst[0], 0);
	mpix_test_equal(dst[1], 0);
	mpix_test_equal(dst[2], src[2]);
}

void test_grayscale_matrix(void)
{
	union mpix_correction_any correction = {
		.color_matrix.levels  = {
			/* 1st matrix row */
			0.33 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.33 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.33 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 2nd matrix row */
			0.33 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.33 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.33 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 3rd matrix row */
			0.33 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.33 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.33 * (1 << MPIX_CORRECTION_SCALE_BITS),
		},
	};

	mpix_correction_color_matrix_rgb24(src, dst, WIDTH * HEIGHT, 0, &correction);

	mpix_test_equal(dst[0], dst[1]);
	mpix_test_equal(dst[1], dst[2]);
}

void test_extract_red_matrix(void)
{
	union mpix_correction_any correction = {
		.color_matrix.levels  = {
			/* 1st matrix row */
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 2nd matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 3rd matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
		},
	};

	mpix_correction_color_matrix_rgb24(src, dst, WIDTH * HEIGHT, 0, &correction);

	mpix_test_equal(dst[0], src[1]);
	mpix_test_equal(dst[1], 0);
	mpix_test_equal(dst[2], 0);
}

void test_extract_green_matrix(void)
{
	union mpix_correction_any correction = {
		.color_matrix.levels  = {
			/* 1st matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 2nd matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 3rd matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
		},
	};

	mpix_correction_color_matrix_rgb24(src, dst, WIDTH * HEIGHT, 0, &correction);

	mpix_test_equal(dst[0], 0);
	mpix_test_equal(dst[1], src[2]);
	mpix_test_equal(dst[2], 0);
}

void test_extract_blue_matrix(void)
{
	union mpix_correction_any correction = {
		.color_matrix.levels  = {
			/* 1st matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 2nd matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			/* 3rd matrix row */
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			0.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
			1.0 * (1 << MPIX_CORRECTION_SCALE_BITS),
		},
	};

	mpix_correction_color_matrix_rgb24(src, dst, WIDTH * HEIGHT, 0, &correction);

	mpix_test_equal(dst[0], 0);
	mpix_test_equal(dst[1], 0);
	mpix_test_equal(dst[2], src[2]);
}

int main(void)
{
	/* Generate test input data */
	for (uint16_t h = 0; h < HEIGHT; h++) {
		for (uint16_t w = 0; w < WIDTH; w++) {
			src[h * WIDTH * 3 + w * 3 + 0] = w < WIDTH / 2 ? 0x00 : 0xff;
			src[h * WIDTH * 3 + w * 3 + 1] = (h % 3 + w % 3) / 4 * 0xff;
			src[h * WIDTH * 3 + w * 3 + 2] = h * 0xff / HEIGHT;
		}
	}

	test_some_matrix();
	test_identity_matrix();
	test_red_to_gray_matrix();
	test_green_to_gray_matrix();
	test_blue_to_gray_matrix();
	test_grayscale_matrix();
	test_extract_red_matrix();
	test_extract_green_matrix();
	test_extract_blue_matrix();

	return 0;
}
