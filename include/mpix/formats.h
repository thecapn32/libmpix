/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_formats mpix/formats.h
 * @brief Formats definition
 * @{
 */
#ifndef MPIX_FORMAT_H
#define MPIX_FORMAT_H

#include <stdlib.h>
#include <stdint.h>

#include <zephyr/sys/util.h>
#include <zephyr/sys/byteorder.h>

/**
 * @brief Generate a string out of a pixel format.
 *
 * @param fmt Format as a 32-bit four character code.
 * @return A local nul-terminated string for this four character code.
 */
#define MPIX_FMT_TO_STR(fmt) VIDEO_FOURCC_TO_STR(fmt)

/**
 * @brief Define a new pixel format, with defaults for most common values.
 *
 * @param a 1st character of the Four Character Code
 * @param b 2nd character of the Four Character Code
 * @param c 3rd character of the Four Character Code
 * @param d 4rd character of the Four Character Code
 */
#define MPIX_FOURCC(a, b, c, d) VIDEO_FOURCC((a), (b), (c), (d))

static inline uint8_t mpix_bits_per_pixel(uint32_t fourcc)
{
	switch (fourcc) {

#define MPIX_FMT_RGB332 MPIX_FOURCC('R', 'G', 'B', '1')
	case MPIX_FMT_RGB332:
		return 8;

#define MPIX_FMT_RGB565 MPIX_FOURCC('R', 'G', 'B', 'P')
	case MPIX_FMT_RGB565:
		return 16;

#define MPIX_FMT_RGB565X MPIX_FOURCC('R', 'G', 'B', 'R')
	case MPIX_FMT_RGB565X:
		return 16;

#define MPIX_FMT_RGB24 MPIX_FOURCC('R', 'G', 'B', '3')
	case MPIX_FMT_RGB24:
		return 24;

#define MPIX_FMT_YUV12 MPIX_FOURCC('Y', 'U', 'V', 'C')
	case MPIX_FMT_YUV12:
		return 12;

#define MPIX_FMT_YUV24 MPIX_FOURCC('Y', 'U', 'V', '3')
	case MPIX_FMT_YUV24:
		return 24;

#define MPIX_FMT_YUYV MPIX_FOURCC('Y', 'U', 'Y', 'V')
	case MPIX_FMT_YUYV:
		return 16;

#define MPIX_FMT_GREY MPIX_FOURCC('G', 'R', 'E', 'Y')
	case MPIX_FMT_GREY:
		return 8;

#define MPIX_FMT_SRGGB8 MPIX_FOURCC('R', 'G', 'G', 'B')
	case MPIX_FMT_SRGGB8:
		return 8;

#define MPIX_FMT_SBGGR8 MPIX_FOURCC('B', 'G', 'G', 'R')
	case MPIX_FMT_SBGGR8:
		return 8;

#define MPIX_FMT_SGBRG8 MPIX_FOURCC('G', 'B', 'R', 'G')
	case MPIX_FMT_SGBRG8:
		return 8;

#define MPIX_FMT_SGRBG8 MPIX_FOURCC('G', 'R', 'B', 'G')
	case MPIX_FMT_SGRBG8:
		return 8;

#define MPIX_FMT_PALETTE1 MPIX_FOURCC('P', 'L', 'T', '1')
	case MPIX_FMT_PALETTE1:
		return 1;

#define MPIX_FMT_PALETTE2 MPIX_FOURCC('P', 'L', 'T', '2')
	case MPIX_FMT_PALETTE2:
		return 2;

#define MPIX_FMT_PALETTE4 MPIX_FOURCC('P', 'L', 'T', '4')
	case MPIX_FMT_PALETTE4:
		return 4;

#define MPIX_FMT_PALETTE8 MPIX_FOURCC('P', 'L', 'T', '8')
	case MPIX_FMT_PALETTE8:
		return 8;

	default:
		return 0;
	}
}

#endif /** @} */
