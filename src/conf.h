/*
 * https://github.com/mochji/crescent
 * conf.h
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

/*
 * Public build configuration for Crescent
 *
 * Search '@' for all configurable definitions.
 *
 * You should change the definitions here, rather than through -D, as other
 * code connected to Crescent must (mostly) use the same configuration.
 */

#ifndef CRS_CONF_H
#define CRS_CONF_H

#include <limits.h>

/*
 * ============================================================================
 * Version and license configuration
 *
 * These definitions contain stuff such as Crescent version, license
 * information, etc. These do not need to be changed.
 * ============================================================================
 */

#define CRS_VERSION_MAJOR 0
#define CRS_VERSION_MINOR 0
#define CRS_VERSION_PATCH 0

#define CRS_VERSION (CRS_VERSION_MAJOR * 10 + CRS_VERSION_MINOR)
#define CRS_RELEASE (CRS_VERSION * 10 + CRS_VERSION_PATCH)
#define CRS_AUTHORS "mochji"

/*
 * ============================================================================
 * Platform configuration
 *
 * Change these definitions, if needed, to compile Crescent for a specific
 * platform.
 * ============================================================================
 */

#define CRS_32INT (UINT_MAX >> 31)

/*
 * ============================================================================
 * Type configuration
 * ============================================================================
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
 * Change this definition to a non-zero value to restrict Crescent to 32-bit
 * number types, rather than the default 64-bit types.
 */

#define CRS_32BIT 0

/*
 * @ CRS_INTEGER_TYPE
 * @ CRS_FLOAT_TYPE
 *
 * Type of Crescent integers and floats.
 */

#if CRS_32BIT
#	if CRS_32INT
#		define CRS_INTEGER_TYPE CRS_INTEGER_INT
#	else
#		define CRS_INTEGER_TYPE CRS_INTEGER_LONG
#	endif
#	define CRS_FLOAT_TYPE       CRS_FLOAT_FLOAT
#else
#	define CRS_INTEGER_TYPE CRS_INTEGER_LLONG
#	define CRS_FLOAT_TYPE   CRS_FLOAT_DOUBLE
#endif

/*
 * @ CRS_STRING_ALLOCSPACE
 * @ CRS_ARRAY_ALLOCSPACE
 *
 * THESE DEFINITIONS WILL BE REMOVED !!
 */

#define CRS_STRING_ALLOCSPACE 32
#define CRS_ARRAY_ALLOCSPACE 32

/*
 * ============================================================================
 * End of configurable options
 * ============================================================================
 */

#ifdef __cplusplus
#	error Crescent is not yet supported for C++.
#endif

/*
 * The last 3 bits within a type tag encode properties about the type:
 *
 * (1 = yes, 0 = no)
 *
 * - 3rd to last (leftmost) bit: Can you call this type?
 * - 2nd to last (middle) bit:   Can you query the length of this type?
 * - last (rightmost) bit:       Is this type a number?
 */

#define CRS_TYPE_NIL       0  /* 0b00000000 */
#define CRS_TYPE_BOOLEAN   8  /* 0b00001000 */
#define CRS_TYPE_INTEGER   1  /* 0b00000001 */
#define CRS_TYPE_FLOAT     9  /* 0b00001001 */
#define CRS_TYPE_STRING    2  /* 0b00000010 */
#define CRS_TYPE_ARRAY     10 /* 0b00001010 */
#define CRS_TYPE_CFUNCTION 4  /* 0b00000100 */

#define CRS_STATUS_OK    0
#define CRS_STATUS_ERROR 1
#define CRS_STATUS_NOMEM 2

#if CRS_INTEGER_TYPE == CRS_INTEGER_INT
#	define CRS_INTEGER     int
#	define CRS_INTEGER_FMT "%d"
#	define CRS_INTEGER_MAX INT_MAX
#	define CRS_INTEGER_MIN INT_MIN
#elif CRS_INTEGER_TYPE == CRS_INTEGER_LONG
#	define CRS_INTEGER     long
#	define CRS_INTEGER_FMT "%ld"
#	define CRS_INTEGER_MAX LONG_MAX
#	define CRS_INTEGER_MIN LONG_MIN
#elif CRS_INTEGER_TYPE == CRS_INTEGER_LLONG
#	define CRS_INTEGER     long long
#	define CRS_INTEGER_FMT "%lld"
#	define CRS_INTEGER_MAX LLONG_MAX
#	define CRS_INTEGER_MIN LLONG_MIN
#endif

#if CRS_FLOAT_TYPE == CRS_FLOAT_FLOAT
#	define CRS_FLOAT     float
#	define CRS_FLOAT_FMT "%f"
#elif CRS_FLOAT_TYPE == CRS_FLOAT_DOUBLE
#	define CRS_FLOAT     double
#	define CRS_FLOAT_FMT "%lf"
#elif CRS_FLOAT_TYPE == CRS_FLOAT_LDOUBLE
#	define CRS_FLOAT     long double
#	define CRS_FLOAT_FMT "%llf"
#endif

struct crs_State;

typedef CRS_INTEGER crs_Integer;
typedef CRS_FLOAT   crs_Float;
typedef int        (crs_CFunction)(struct crs_State*);

#endif
