/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_port mpix/port.h
 * @brief Functions to implement to port libmpix
 *
 * @{
 */
#ifndef MPIX_PORT
#define MPIX_PORT

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Get the uptime in microsecond, used to compute performance statistics
 *
 * Counter overflows are not fatal to the system as this is only for benchmarking purpose.
 *
 * @return The 32-bit counter of microsecond since boot
 */
uint32_t mpix_port_get_uptime_us(void);

/**
 * @brief Allocate a buffer to use with libmpix
 *
 * This will be used to allocate the small intermediate buffers present between the operations.
 *
 * @param size Number of bytes available in the buffer
 * @return The pointer to the new buffer or NULL if allocation failed.
 */
void *mpix_port_alloc(size_t size);

/**
 * @brief Free a buffer allocated with @ref mpix_port_alloc().
 *
 * This will be used to free the buffers that were allocated after all processing.
 *
 * @param mem Pointer to the buffer to free.
 */
void mpix_port_free(void *mem);

/**
 * @brief Print debug information to the console
 *
 * This will be used to log debug messages according to the log level, as well as print image
 * previews in the terminal.
 *
 * @param mem Pointer to the buffer to free.
 */
void mpix_port_printf(const char *fmt, ...);

#endif /** @} */
