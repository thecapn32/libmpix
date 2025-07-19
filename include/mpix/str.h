/**
 * SPDX-License-Identifier: Apache-2.0
 * @internal
 * @{
 */
#ifndef MPIX_STR_H
#define MPIX_STR_H

#include <stdint.h>
#include <errno.h>
#include <string.h>

struct mpix_str {
	const char *name;
	uint32_t value;
};

extern const struct mpix_str mpix_str_fmt[];
extern const struct mpix_str mpix_str_kernel[];
extern const struct mpix_str mpix_str_resize[];
extern const struct mpix_str mpix_str_correction[];

static inline int mpix_str_get_value(const struct mpix_str *table, const char *name, uint32_t *val)
{
	for (size_t i = 0; table[i].name != NULL; i++) {
		if (strcmp(table[i].name, name) == 0) {
			*val = table[i].value;
			return 0;
		}
	}

	return -EINVAL;
}

#endif /* @} */
