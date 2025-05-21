/**
 * SPDX-License-Identifier: Apache-2.0
 * @defgroup mpix_image mpix/image.h
 * @brief User API
 * @{
 */
#ifndef MPIX_PORT
#define MPIX_PORT

#include <stddef.h>
#include <stdint.h>

uint32_t mpix_port_get_uptime_us(void);

void *mpix_port_alloc(size_t size);

void mpix_port_free(void *mem);

#endif /** @} */
