/*
 * https://github.com/mochji/crescent
 * limit.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_LIMIT_H
#define CRS_LIMIT_H

#include <stdint.h>
#include <limits.h>

#include "conf.h"

/*
 * Maximum theoretical size for an object--i.e. the value returned by the
 * length operator--which must fit within a size_t and crs_Integer. However,
 * the actual maximum size for an object is limited by the specifics of that
 * type.
 */
#define CRS_MAX_LENGTH          \
	(SIZE_MAX > CRS_INTEGER_MAX \
		? CRS_INTEGER_MAX       \
		: SIZE_MAX)

/* global to entire stack */
#define CRS_MIN_STACK 64

/* local to a stack frame */
#define CRS_MIN_TOP 16
#define CRS_MAX_TOP INT_MAX

#define CRS_MAX_CALLS  4096
#define CRS_MAX_CCALLS 200

#define CRS_MAX_LOCALS 200

typedef unsigned char crs_byte;

#if CRS_32INT
typedef unsigned long crs_mem;
#define CRS_MAX_MEM ULONG_MAX
#else
typedef size_t crs_mem;
#define CRS_MAX_MEM SIZE_MAX
#endif

/* useful bit macros */
#define bit_mask(b)         (1 << (b))
#define bit_get(x, m)       ((x) & (m))
#define bit_set(x, m)       ((x) | (m))
#define bit_reset(x, m)     ((x) & ~(m))
#define bit_change(x, v, m) (((x) & ~(m)) | (v))
#define bit_1mask(o, l)     ((bit_mask(l) - 1) << (o))

#endif
