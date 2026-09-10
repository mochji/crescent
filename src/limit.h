/*
 * https://github.com/mochji/crescent
 * limit.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_LIMIT_H
#define CRS_LIMIT_H

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#include "crescent/conf.h"

#if CRS_INTEGER_MAX > SIZE_MAX
#define CRS_MAX_LENGTH SIZE_MAX
#else
#define CRS_MAX_LENGTH CRS_INTEGER_MAX
#endif

/* global to entire stack */
#define CRS_MIN_STACK 64
#define CRS_MIN_FREE  3 /* for internal use of stack */

/* local to a stack frame */
#define CRS_MIN_TOP 16
#define CRS_MAX_TOP 10000

#define CRS_MAX_LEVEL SHRT_MAX

#define MAX_LOCALS 200 /* reserve some space for temporary registers */
#define MAX_REGS   255 /* 255 is reserved in some cases, anyways     */
#define MAX_RET    255

#if CRS_32INT
typedef int      crs_s32;
typedef unsigned crs_u32;
typedef size_t   crs_mem;
typedef size_t   crs_uptr;
#define CRS_MAX_U32 UINT_MAX
#define CRS_MAX_MEM SIZE_MAX
#else
typedef long          crs_s32;
typedef unsigned long crs_u32;
typedef unsigned long crs_mem;
typedef unsigned long crs_uptr; /* size_t may be smaller than a pointer */
#define CRS_MAX_U32 ULONG_MAX
#define CRS_MAX_MEM ULONG_MAX
#endif

typedef unsigned char crs_byte;
typedef crs_u32       crs_instr;

/* minimum buffer size needed to format a number */
#define CRS_MAX_FMTNUM 64

/* useful bit macros */
#define bit_mask(b)         (1 << (b))
#define bit_get(x, m)       ((x) & (m))
#define bit_set(x, m)       ((x) | (m))
#define bit_reset(x, m)     ((x) & ~(m))
#define bit_change(x, v, m) (((x) & ~(m)) | (v))
#define bit_1mask(o, l)     (((1 << (l)) - 1) << (o))

#ifdef CRS_DEBUG
#undef NDEBUG
#include <assert.h>
#else
#define NDEBUG
#undef assert
#define assert(c) ((void)0)
#endif

#define UNUSED(v) ((void)(v))

#endif
