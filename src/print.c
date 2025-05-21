/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#include <mpix/op_convert.h>
#include <mpix/print.h>
#include <mpix/utils.h>

static uint8_t mpix_rgb24_to_256color(const uint8_t rgb24[3])
{
	return 16 + rgb24[0] * 6 / 256 * 36 + rgb24[1] * 6 / 256 * 6 + rgb24[2] * 6 / 256 * 1;
}

static uint8_t mpix_gray8_to_256color(uint8_t gray8)
{
	return 232 + gray8 * 24 / 256;
}

static void mpix_print_truecolor(const uint8_t row0[3], const uint8_t row1[3])
{
	printf("\e[48;2;%u;%u;%um\e[38;2;%u;%u;%um▄",
		row0[0], row0[1], row0[2], row1[0], row1[1], row1[2]);
}

static void mpix_print_256color(const uint8_t row0[3], const uint8_t row1[3])
{
	printf("\e[48;5;%um\e[38;5;%um▄",
		mpix_rgb24_to_256color(row0), mpix_rgb24_to_256color(row1));
}

static void mpix_print_256gray(uint8_t row0, uint8_t row1)
{
	printf("\e[48;5;%um\e[38;5;%um▄",
		mpix_gray8_to_256color(row0), mpix_gray8_to_256color(row1));
}

typedef void fn_print_t(const uint8_t row0[3], const uint8_t row1[3]);

typedef void fn_conv_t(const uint8_t *src, uint8_t *dst, uint16_t w);

static void mpix_print(const uint8_t *src, size_t size, uint16_t width, uint16_t height,
			fn_print_t *fn_print, fn_conv_t *fn_conv, int bitspp, int npix)
{
	size_t pitch = width * bitspp / BITS_PER_BYTE;
	uint8_t nbytes = npix * bitspp  / BITS_PER_BYTE;

	for (size_t i = 0, h = 0; h + 2 <= height; h += 2) {
		for (size_t w = 0; w + npix <= width; w += npix, i += nbytes) {
			uint8_t rgb24a[3 * 2], rgb24b[3 * 2];

			assert(npix <= 2);

			fn_conv(&src[i + pitch * 0], rgb24a, npix);
			fn_conv(&src[i + pitch * 1], rgb24b, npix);

			if (i + pitch > size) {
				printf("\e[m *** early end of buffer at %zu bytes ***\n",
					    size);
				return;
			}

			for (int n = 0; n < npix; n++) {
				fn_print(&rgb24a[n * 3], &rgb24b[n * 3]);
			}
		}
		printf("\e[m|\n");

		/* Skip the second h being printed at the same time */
		i += pitch;
	}
}

static void mpix_print_buffer(const uint8_t *buffer, size_t size, uint16_t width, uint16_t height,
			       uint32_t fourcc, fn_print_t *fn)
{
	switch (fourcc) {
	case MPIX_FMT_RGB24:
		mpix_print(buffer, size, width, height, fn, mpix_convert_rgb24_to_rgb24,
			    mpix_bits_per_pixel(fourcc), 1);
		break;
	case MPIX_FMT_RGB565:
		mpix_print(buffer, size, width, height, fn, mpix_convert_rgb565le_to_rgb24,
			    mpix_bits_per_pixel(fourcc), 1);
		break;
	case MPIX_FMT_RGB565X:
		mpix_print(buffer, size, width, height, fn, mpix_convert_rgb565be_to_rgb24,
			    mpix_bits_per_pixel(fourcc), 1);
		break;
	case MPIX_FMT_RGB332:
		mpix_print(buffer, size, width, height, fn, mpix_convert_rgb332_to_rgb24,
			    mpix_bits_per_pixel(fourcc), 1);
		break;
	case MPIX_FMT_YUYV:
		mpix_print(buffer, size, width, height, fn, mpix_convert_yuyv_to_rgb24_bt709,
			    mpix_bits_per_pixel(fourcc), 2);
		break;
	case MPIX_FMT_YUV24:
		mpix_print(buffer, size, width, height, fn, mpix_convert_yuv24_to_rgb24_bt709,
			    mpix_bits_per_pixel(fourcc), 1);
		break;
	case MPIX_FMT_SRGGB8:
	case MPIX_FMT_SBGGR8:
	case MPIX_FMT_SGBRG8:
	case MPIX_FMT_SGRBG8:
	case MPIX_FMT_GREY:
		mpix_print(buffer, size, width, height, fn, mpix_convert_y8_to_rgb24_bt709,
			   mpix_bits_per_pixel(fourcc), 1);
		break;
	default:
		printf("Printing %s buffers not supported\n", MPIX_FOURCC_TO_STR(fourcc));
	}
}

void mpix_print_buffer_truecolor(const uint8_t *buffer, size_t size, uint16_t width,
				  uint16_t height, uint32_t fourcc)
{
	mpix_print_buffer(buffer, size, width, height, fourcc, mpix_print_truecolor);
}

void mpix_print_buffer_256color(const uint8_t *buffer, size_t size, uint16_t width,
				  uint16_t height, uint32_t fourcc)
{
	mpix_print_buffer(buffer, size, width, height, fourcc, mpix_print_256color);
}

void mpix_image_print_truecolor(struct mpix_image *img)
{
	mpix_print_buffer_truecolor(img->buffer, img->size, img->width, img->height, img->format);
}

void mpix_image_print_256color(struct mpix_image *img)
{
	mpix_print_buffer_256color(img->buffer, img->size, img->width, img->height, img->format);
}

void mpix_hexdump_raw8(const uint8_t *raw8, size_t size, uint16_t width, uint16_t height)
{
	for (uint16_t h = 0; h < height; h++) {
		for (uint16_t w = 0; w < width; w++) {
			size_t i = h * width * 1 + w * 1;

			if (i >= size) {
				printf("\e[m *** early end of buffer at %zu bytes ***\n",
					    size);
				return;
			}

			printf(" %02x", raw8[i]);
		}
		printf(" row%u\n", h);
	}
}

void mpix_hexdump_rgb24(const uint8_t *rgb24, size_t size, uint16_t width, uint16_t height)
{
	printf(" ");
	for (uint16_t w = 0; w < width; w++) {
		printf("col%-7u", w);
	}
	printf("\n");

	for (uint16_t w = 0; w < width; w++) {
		printf(" R  G  B  ");
	}
	printf("\n");

	for (uint16_t h = 0; h < height; h++) {
		for (uint16_t w = 0; w < width; w++) {
			size_t i = h * width * 3 + w * 3;

			if (i + 2 >= size) {
				printf("\e[m *** early end of buffer at %zu bytes ***\n",
					    size);
				return;
			}

			printf(" %02x %02x %02x ", rgb24[i + 0], rgb24[i + 1], rgb24[i + 2]);
		}
		printf(" row%u\n", h);
	}
}

void mpix_hexdump_rgb565(const uint8_t *rgb565, size_t size, uint16_t width, uint16_t height)
{
	printf(" ");
	for (uint16_t w = 0; w < width; w++) {
		printf("col%-4u", w);
	}
	printf("\n");

	for (uint16_t w = 0; w < width; w++) {
		printf(" RGB565");
	}
	printf("\n");

	for (uint16_t h = 0; h < height; h++) {
		for (uint16_t w = 0; w < width; w++) {
			size_t i = h * width * 2 + w * 2;

			if (i + 1 >= size) {
				printf("\e[m *** early end of buffer at %zu bytes ***\n",
					    size);
				return;
			}

			printf(" %02x %02x ", rgb565[i + 0], rgb565[i + 1]);
		}
		printf(" row%u\n", h);
	}
}

void mpix_hexdump_yuyv(const uint8_t *yuyv, size_t size, uint16_t width, uint16_t height)
{
	printf(" ");
	for (uint16_t w = 0; w < width; w++) {
		printf("col%-3u", w);
		if ((w + 1) % 2 == 0) {
			printf(" ");
		}
	}
	printf("\n");

	for (uint16_t w = 0; w < width; w++) {
		printf(" %c%u", "YUYV"[w % 2 * 2 + 0], w % 2);
		printf(" %c%u", "YUYV"[w % 2 * 2 + 1], w % 2);
		if ((w + 1) % 2 == 0) {
			printf(" ");
		}
	}
	printf("\n");

	for (uint16_t h = 0; h < height; h++) {
		for (uint16_t w = 0; w < width; w++) {
			size_t i = h * width * 2 + w * 2;

			if (i + 1 >= size) {
				printf("\e[m *** early end of buffer at %zu bytes ***\n",
					    size);
				return;
			}

			printf(" %02x %02x", yuyv[i], yuyv[i + 1]);
			if ((w + 1) % 2 == 0) {
				printf(" ");
			}
		}
		printf(" row%u\n", h);
	}
}

static void mpix_print_hist_scale(size_t size)
{
	for (uint16_t i = 0; i < size; i++) {
		mpix_print_256gray(0, i * 256 / size);
	}
	printf("\e[m\n");
}

void mpix_print_rgb24hist(const uint16_t *rgb24hist, size_t size, uint16_t height)
{
	const uint16_t *r8hist = &rgb24hist[size / 3 * 0];
	const uint16_t *g8hist = &rgb24hist[size / 3 * 1];
	const uint16_t *b8hist = &rgb24hist[size / 3 * 2];
	uint32_t max = 1;

	assert(size % 3 == 0 /* Each of R, G, B channel should have the same size */);

	for (size_t i = 0; i < size; i++) {
		max = rgb24hist[i] > max ? rgb24hist[i] : max;
	}

	for (uint16_t h = height; h > 1; h--) {
		for (size_t i = 0; i < size / 3; i++) {
			uint8_t row0[3];
			uint8_t row1[3];

			row0[0] = (r8hist[i] * height / max > h - 0) ? 0xff : 0x00;
			row0[1] = (g8hist[i] * height / max > h - 0) ? 0xff : 0x00;
			row0[2] = (b8hist[i] * height / max > h - 0) ? 0xff : 0x00;
			row1[0] = (r8hist[i] * height / max > h - 1) ? 0xff : 0x00;
			row1[1] = (g8hist[i] * height / max > h - 1) ? 0xff : 0x00;
			row1[2] = (b8hist[i] * height / max > h - 1) ? 0xff : 0x00;

			mpix_print_256color(row0, row1);
		}
		printf("\e[m| - %u\n", h * max / height);
	}

	mpix_print_hist_scale(size / 3);
}

void mpix_print_y8hist(const uint16_t *y8hist, size_t size, uint16_t height)
{
	uint32_t max = 1;

	for (size_t i = 0; i < size; i++) {
		max = y8hist[i] > max ? y8hist[i] : max;
	}

	for (uint16_t h = height; h > 1; h--) {
		for (size_t i = 0; i < size; i++) {
			uint8_t row0 = (y8hist[i] * height / max > h - 0) ? 0xff : 0x00;
			uint8_t row1 = (y8hist[i] * height / max > h - 1) ? 0xff : 0x00;

			mpix_print_256gray(row0, row1);
		}
		printf("\e[m| - %u\n", h * max / height);
	}

	mpix_print_hist_scale(size);
}
