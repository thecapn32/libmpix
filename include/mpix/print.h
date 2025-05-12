/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_print mpix/print.h
 * @brief Print images and statistics
 * @{
 */
#ifndef MPIX_PRINT_H
#define MPIX_PRINT_H

#include <stddef.h>
#include <stdint.h>

#include <zephyr/shell/shell.h>

/**
 * @brief Print a buffer using higher quality TRUECOLOR terminal escape codes.
 *
 * @param buf Imagme buffer to display in the terminal.
 * @param size Size of the buffer in bytes.
 * @param width Number of pixel of the input buffer in width
 * @param height Max number of rows to print
 * @param fourcc Format of the buffer to print
 */
void mpix_print_buffer_truecolor(const uint8_t *buf, size_t size, uint16_t width,
				  uint16_t height, uint32_t fourcc);
/**
 * @brief Print a buffer using higher speed 256COLOR terminal escape codes.
 * @copydetails mpix_print_buffer_truecolor()
 */
void mpix_print_buffer_256color(const uint8_t *buf, size_t size, uint16_t width,
				 uint16_t height, uint32_t fourcc);

/**
 * @brief Hexdump a buffer in the RAW8 format
 *
 * @param buf Input buffer to display in the terminal.
 * @param size Size of the input buffer in bytes.
 * @param width Number of pixel of the input buffer in width
 * @param height Max number of rows to print
 */
void mpix_hexdump_raw8(const uint8_t *buf, size_t size, uint16_t width, uint16_t height);
/**
 * @brief Hexdump a buffer in the RGB24 format
 * @copydetails mpix_hexdump_raw8()
 */
void mpix_hexdump_rgb24(const uint8_t *buf, size_t size, uint16_t width, uint16_t height);
/**
 * @brief Hexdump a buffer in the RGB565 format
 * @copydetails mpix_hexdump_raw8()
 */
void mpix_hexdump_rgb565(const uint8_t *buf, size_t size, uint16_t width, uint16_t height);
/**
 * @brief Hexdump a buffer in the YUYV format
 * @copydetails mpix_hexdump_raw8()
 */
void mpix_hexdump_yuyv(const uint8_t *buf, size_t size, uint16_t width, uint16_t height);

/**
 * @brief Printing RGB histograms to the terminal.
 *
 * @param rgb24hist Buffer storing 3 histograms one after the other, for the R, G, B channels.
 * @param size Total number of buckets in total contained within @p rgb24hist all channels included.
 * @param height Desired height of the chart in pixels.
 */
void mpix_print_rgb24hist(const uint16_t *rgb24hist, size_t size, uint16_t height);

/**
 * @brief Printing Y histograms to the terminal.
 *
 * @param y8hist Buffer storing the histogram for the Y (luma) channel.
 * @param size Total number of buckets in total contained within @p hist.
 * @param height Desired height of the chart in pixels.
 */
void mpix_print_y8hist(const uint16_t *y8hist, size_t size, uint16_t height);

/**
 * @brief Set the shell instance to use when printing via the shell back-end.
 *
 * @see CONFIG_MPIX_PRINT
 *
 * @param sh Shell instance set as a global variable.
 */
void mpix_print_set_shell(struct shell *sh);

#endif /** @} */
