/*
 * https://github.com/mochji/crescent
 * crescent/conf.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

/*
 * Public build configuration for Crescent
 *
 * Search '@' for all configurable definitions.
 *
 * All definitions should be changed through this file, as all code connected to
 * Crescent must (mostly) use the same configuration. The default configuration
 * should work fine, though.
 */

#ifndef CRS_CRESCENT_CONF_H
#define CRS_CRESCENT_CONF_H

#include <limits.h>

/*
 * =============================================================================
 * Version and license configuration
 *
 * These definitions contain stuff such as Crescent version, license
 * information, etc. These do not need to be changed.
 * =============================================================================
 */

#define CRS_VERSION_MAJOR 0
#define CRS_VERSION_MINOR 0
#define CRS_VERSION_PATCH 0

#define CRS_VERSION (CRS_VERSION_MAJOR * 10 + CRS_VERSION_MINOR)
#define CRS_RELEASE (CRS_VERSION * 10 + CRS_VERSION_PATCH)
#define CRS_AUTHORS "mochji"

/*
 * =============================================================================
 * Platform configuration
 *
 * Change these definitions, if needed, to compile Crescent for a specific
 * platform.
 * =============================================================================
 */

#define CRS_32INT (UINT_MAX >> 31)

/*
 * =============================================================================
 * Type configuration
 * =============================================================================
 */

#define CRS_INTEGER_INT   0
#define CRS_INTEGER_LONG  1
#define CRS_INTEGER_LLONG 2

#define CRS_FLOAT_FLOAT   0
#define CRS_FLOAT_DOUBLE  1
#define CRS_FLOAT_LDOUBLE 2

/*
 * @ CRS_32BIT
 *
 * If defined, restrict Crescent to 32-bit number types, rather than the default
 * 64-bit types.
 */

/* #define CRS_32BIT */

/*
 * @ CRS_INTEGER_TYPE
 * @ CRS_FLOAT_TYPE
 * @ CRS_DAPFLOAT_TYPE
 *
 * Type of Crescent integers and floats. CRS_DAPFLOAT_TYPE specifies the default
 * argument promotion for floats.
 */

#ifdef CRS_32BIT
#if CRS_32INT
#define CRS_INTEGER_TYPE CRS_INTEGER_INT
#else
#define CRS_INTEGER_TYPE CRS_INTEGER_LONG
#endif
#define CRS_FLOAT_TYPE CRS_FLOAT_FLOAT
#else
#define CRS_INTEGER_TYPE CRS_INTEGER_LLONG
#define CRS_FLOAT_TYPE CRS_FLOAT_DOUBLE
#endif

#define CRS_DAPFLOAT_TYPE CRS_FLOAT_DOUBLE

/*
 * @ CRS_STR2NUM
 *
 * If defined, perform automatic type coercion from strings to numbers (equality
 * doesn't perform type coercion).
 */

/* #define CRS_STR2NUM */

/*
 * =============================================================================
 * End of configurable options
 * =============================================================================
 */

enum {
	CRS_OP_UNM,
	CRS_OP_ADD,
	CRS_OP_SUB,
	CRS_OP_MUL,
	CRS_OP_DIV,
	CRS_OP_POW,
	CRS_OP_MOD,
	CRS_OP_NOT,
	CRS_OP_AND,
	CRS_OP_OR,
	CRS_OP_XOR,
	CRS_OP_SHL,
	CRS_OP_SHR,
	CRS_OP_EQ,
	CRS_OP_LT,
	CRS_OP_LE
};

/*
 * The lower 4 bits within a type tag encode some properties of the type, most
 * of which are for the garbage collector.
 *
 * (1 = yes, 0 = no)
 *
 * bit 0 (1): Is this type a number?
 * bit 1 (2): Is this type collectable?
 * bit 2 (4): Can this type reference objects? (a)
 * bit 3 (8): Should this type be traversed atomically? (b)
 *
 * The high 4 differentiate it from other types with the same properties.
 *
 * a: If a type cannot reference objects, there's no use in marking it gray.
 * b: For types without write barriers: they should be traversed atomically.
 */

#define CRS_TYPE_NIL        0 /* 0000 0000 */
#define CRS_TYPE_BOOLEAN   16 /* 0001 0000 */
#define CRS_TYPE_INTEGER    1 /* 0000 0001 */
#define CRS_TYPE_FLOAT     17 /* 0001 0001 */
#define CRS_TYPE_CFUNCTION 32 /* 0010 0000 */
#define CRS_TYPE_STRING     2 /* 0000 0010 */
#define CRS_TYPE_TABLE      6 /* 0000 0110 */
#define CRS_TYPE_FUNCTION  22 /* 0001 0110 */
#define CRS_TYPE_THREAD    14 /* 0000 1110 */

#define CRS_STATUS_OK      0 /* no error               */
#define CRS_STATUS_ERROR   1 /* runtime error          */
#define CRS_STATUS_CODEERR 2 /* load/compilation error */
#define CRS_STATUS_MEMERR  3 /* out of memory          */

#if CRS_INTEGER_TYPE == CRS_INTEGER_INT
#define CRS_INTEGER     int
#define CRS_INTEGER_FMT "%d"
#define CRS_INTEGER_MAX INT_MAX
#define CRS_INTEGER_MIN INT_MIN
#define CRS_UNSIGNED     unsigned
#define CRS_UNSIGNED_FMT "%u"
#define CRS_UNSIGNED_MAX UINT_MAX
#elif CRS_INTEGER_TYPE == CRS_INTEGER_LONG
#define CRS_INTEGER     long
#define CRS_INTEGER_FMT "%ld"
#define CRS_INTEGER_MAX LONG_MAX
#define CRS_INTEGER_MIN LONG_MIN
#define CRS_UNSIGNED     unsigned long
#define CRS_UNSIGNED_FMT "%lu"
#define CRS_UNSIGNED_MAX ULONG_MAX
#elif CRS_INTEGER_TYPE == CRS_INTEGER_LLONG
#define CRS_INTEGER     long long
#define CRS_INTEGER_FMT "%lld"
#define CRS_INTEGER_MAX LLONG_MAX
#define CRS_INTEGER_MIN LLONG_MIN
#define CRS_UNSIGNED     unsigned long long
#define CRS_UNSIGNED_FMT "%llu"
#define CRS_UNSIGNED_MAX ULLONG_MAX
#endif

#if CRS_FLOAT_TYPE == CRS_FLOAT_FLOAT
#define CRS_FLOAT     float
#define CRS_FLOAT_FMT "%f"
#elif CRS_FLOAT_TYPE == CRS_FLOAT_DOUBLE
#define CRS_FLOAT     double
#define CRS_FLOAT_FMT "%lf"
#elif CRS_FLOAT_TYPE == CRS_FLOAT_LDOUBLE
#define CRS_FLOAT     long double
#define CRS_FLOAT_FMT "%Lf"
#endif

#if CRS_DAPFLOAT_TYPE == CRS_FLOAT_FLOAT
#define CRS_DAPFLOAT     float
#define CRS_DAPFLOAT_FMT "%f"
#elif CRS_DAPFLOAT_TYPE == CRS_FLOAT_DOUBLE
#define CRS_DAPFLOAT     double
#define CRS_DAPFLOAT_FMT "%lf"
#elif CRS_DAPFLOAT_TYPE == CRS_FLOAT_LDOUBLE
#define CRS_DAPFLOAT     long double
#define CRS_DAPFLOAT_FMT "%Lf"
#endif

/* binary dump signatures */
#define CRS_SIGNATURE "\x7F" "CRS"
#define CRS_DUMPCHECK "\0\r\n\b\x7F\xFF\n\r"

struct crs_Thread;
typedef struct crs_Thread crs_Thread;
typedef int  (crs_Reader)(crs_Thread*, void*, char*, int);
typedef void (crs_Writer)(crs_Thread*, void*, char*, int);

typedef CRS_INTEGER  crs_Integer;
typedef CRS_UNSIGNED crs_Unsigned;
typedef CRS_FLOAT    crs_Float;
typedef CRS_DAPFLOAT crs_DAPFloat;
typedef int         (crs_CFunction)(struct crs_Thread*);

#endif
