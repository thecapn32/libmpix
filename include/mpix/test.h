/**
 * SPDX-License-Identifier: Apache-2.0
 * @internal
 * @defgroup mpix_test mpix/test.h
 * @brief Internal test utilities
 * @{
 */
#ifndef MPIX_TEST_H
#define MPIX_TEST_H

#include <assert.h>

#define mpix_test_ok(ret) \
	assert((ret) == 0)

#define mpix_test_within(val, reference, margin) \
	(assert((val) > (reference) - (margin)), assert((val) < (reference) + (margin)))

#endif /* @} */
