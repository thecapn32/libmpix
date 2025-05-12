# SPDX-License-Identifier: Apache-2.0

#include <zephyr/linker/iterable_sections.h>

ITERABLE_SECTION_RAM(mpix_convert, Z_LINK_ITERABLE_SUBALIGN)
ITERABLE_SECTION_RAM(mpix_resize, Z_LINK_ITERABLE_SUBALIGN)
ITERABLE_SECTION_RAM(mpix_kernel, Z_LINK_ITERABLE_SUBALIGN)
