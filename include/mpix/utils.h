/**
 * SPDX-License-Identifier: Apache-2.0
 * @internal
 * @{
 */
#ifndef MPIX_UTILS_H
#define MPIX_UTILS_H

#include <mpix/config.h>

/* Logging utilities */

#if CONFIG_MPIX_LOG_LEVEL >= 1
#define MPIX_ERR(fmt, ...) printf("ERR: %s: " fmt "\n", __func__, ## __VA_ARGS__)
#else
#define MPIX_ERR(fmt, ...) ((void)0)
#endif

#if CONFIG_MPIX_LOG_LEVEL >= 2
#define MPIX_WRN(fmt, ...) printf("WRN: %s: " fmt "\n", __func__, ## __VA_ARGS__)
#else
#define MPIX_WRN(fmt, ...) ((void)0)
#endif

#if CONFIG_MPIX_LOG_LEVEL >= 3
#define MPIX_INF(fmt, ...) printf("INF: %s: " fmt "\n", __func__, ## __VA_ARGS__)
#else
#define MPIX_INF(fmt, ...) ((void)0)
#endif

#if CONFIG_MPIX_LOG_LEVEL >= 4
#define MPIX_DBG(fmt, ...) printf("DBG: %s: " fmt "\n", __func__, ## __VA_ARGS__)
#else
#define MPIX_DBG(fmt, ...) ((void)0)
#endif

/* Math operations */

#ifndef BITS_PER_BYTE
#define BITS_PER_BYTE 8
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*(arr)))
#endif

#ifndef CLAMP
#define CLAMP(n, min, max) ((n) < (min) ? (min) : (n) > (max) ? (max) : (n))
#endif

#ifndef LOG2
#define _LOG2D(x) (32 - __builtin_clz(x) - 1)
#define _LOG2Q(x) (64 - __builtin_clzll(x) - 1)
#define _LOG2(x) (sizeof(__typeof__(x)) > 4 ? _LOG2Q(x) : _LOG2D(x))
#define LOG2(x) ((x) < 1 ? -1 : _LOG2(x))
#endif

/* Endianness operations */

static inline uint16_t mpix_bswap16(uint16_t i)
{
	return	(i & 0xff00 >> 8) |
		(i & 0x00ff << 8);
}

static inline uint32_t mpix_bswap32(uint32_t i)
{
	return	(i & 0xff000000UL >> 24) | (i & 0x000000ffUL << 24) |
		(i & 0x00ff0000UL >> 8)  | (i & 0x0000ff00UL << 8);
}

static inline uint64_t mpix_bswap64(uint64_t i)
{
	return	(i & 0xff00000000000000ULL >> 56) | (i & 0x00000000000000ffULL << 56) |
		(i & 0x00ff000000000000ULL >> 40) | (i & 0x000000000000ff00ULL << 40) |
		(i & 0x0000ff0000000000ULL >> 24) | (i & 0x0000000000ff0000ULL << 24) |
		(i & 0x000000ff00000000ULL >> 8)  | (i & 0x00000000ff000000ULL << 8);
}

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

#define MPIX_CONV_BE(bits, u) (u)
#define MPIX_CONV_LE(bits, u) mpix_bswap##bits(u)

#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define MPIX_CONV_BE(bits, u) mpix_bswap##bits(u)
#define MPIX_CONV_LE(bits, u) (u)

#endif

static inline uint16_t mpix_htobe16(uint16_t u)
{
	return MPIX_CONV_BE(16, u);
}

static inline uint16_t mpix_htole16(uint16_t u)
{
	return MPIX_CONV_LE(16, u);
}

static inline uint16_t mpix_be16toh(uint16_t u)
{
	return MPIX_CONV_BE(16, u);
}

static inline uint16_t mpix_le16toh(uint16_t u)
{
	return MPIX_CONV_LE(16, u);
}

static inline uint32_t mpix_htobe32(uint32_t u)
{
	return MPIX_CONV_BE(32, u);
}

static inline uint32_t mpix_htole32(uint32_t u)
{
	return MPIX_CONV_LE(32, u);
}

static inline uint32_t mpix_be32toh(uint32_t u)
{
	return MPIX_CONV_BE(32, u);
}

static inline uint32_t mpix_le32toh(uint32_t u)
{
	return MPIX_CONV_LE(32, u);
}

static inline uint64_t mpix_htobe64(uint64_t u)
{
	return MPIX_CONV_BE(64, u);
}

static inline uint64_t mpix_htole64(uint64_t u)
{
	return MPIX_CONV_LE(64, u);
}

static inline uint64_t mpix_be64toh(uint64_t u)
{
	return MPIX_CONV_BE(64, u);
}

static inline uint64_t mpix_le64toh(uint64_t u)
{
	return MPIX_CONV_LE(64, u);
}

#endif /* @} */
