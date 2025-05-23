/* SPDX-License-Identifier: Apache-2.0 */

#include <zephyr/kernel.h>

K_HEAP_DEFINE(mpix_heap, CONFIG_MPIX_HEAP_SIZE);

void *mpix_port_alloc(size_t size)
{
	return k_heap_alloc(&mpix_heap, size, K_NO_WAIT);
}

void mpix_port_free(void *mem)
{
	return k_heap_free(&mpix_heap, mem);
}
