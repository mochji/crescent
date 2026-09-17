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

/*
 * The upper four bits of a type tag encode some properties of the type, most of
 * which are for the garbage collector. Only the lower four bits are exposed to
 * the C API.
 *
 * - bit 4: is number
 * - bit 5: is collectable (gc-managed)
 * - bit 6: can reference other objects (should be marked gray)
 * - bit 7: reserved
 */

#define CRS_TYPE_NIL       (0x00 | CRS_TNIL)       /* 0000 */
#define CRS_TYPE_BOOLEAN   (0x00 | CRS_TBOOLEAN)   /* 0000 */
#define CRS_TYPE_INTEGER   (0x10 | CRS_TINTEGER)   /* 0001 */
#define CRS_TYPE_FLOAT     (0x10 | CRS_TFLOAT)     /* 0001 */
#define CRS_TYPE_CFUNCTION (0x00 | CRS_TCFUNCTION) /* 0000 */
#define CRS_TYPE_STRING    (0x20 | CRS_TSTRING)    /* 0010 */
#define CRS_TYPE_TABLE     (0x60 | CRS_TTABLE)     /* 0110 */
#define CRS_TYPE_FUNCTION  (0x60 | CRS_TFUNCTION)  /* 0110 */
#define CRS_TYPE_THREAD    (0x60 | CRS_TTHREAD)    /* 0110 */
#define CRS_TYPE_USERDATA  (0x60 | CRS_TUSERDATA)  /* 0110 */

#if CRS_INTEGER_MAX > SIZE_MAX
#define CRS_MAX_LENGTH SIZE_MAX
#else
#define CRS_MAX_LENGTH CRS_INTEGER_MAX
#endif

/* global to entire stack */
#define CRS_MIN_STACK 64
#define CRS_MIN_FREE  4 /* for internal use of stack */

/* local to a stack frame */
#define CRS_MIN_TOP 16
#define CRS_MAX_TOP 10000

#define CRS_MAX_LEVEL SHRT_MAX

#define MAX_LOCALS 200 /* reserve some space for temporary registers */
#define MAX_REGS   255 /* 255 is reserved in some cases, anyways     */
#define MAX_RET    CRS_MAX_TOP

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
