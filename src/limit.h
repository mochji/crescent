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

#define CRS_MAX_SIZE           \
	SIZE_MAX > CRS_INTEGER_MAX \
		? CRS_INTEGER_MAX      \
		: SIZE_MAX

/* global to entire stack */
#define CRS_MIN_STACK 64
#define CRS_MAX_STACK SIZE_MAX

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

#define CRS_MAX_SWEEP 50

/* useful bit macros */
#define bit_mask(b)         (1 << (b))
#define bit_get(x, m)       ((x) & (m))
#define bit_set(x, m)       ((x) | (m))
#define bit_reset(x, m)     ((x) & ~(m))
#define bit_change(x, v, m) (((x) & ~(m)) | (v))

#endif
