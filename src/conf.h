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

#ifndef CRESCENT_CONF_H
#define CRESCENT_CONF_H

#include <limits.h>

/*
 * ============================================================================
 * Version and license configuration
 *
 * These definitions contain stuff such as Crescent version, license
 * information, etc. These do not need to be changed.
 * ============================================================================
 */

#define CRESCENT_VERSION_MAJOR 0
#define CRESCENT_VERSION_MINOR 0
#define CRESCENT_VERSION_PATCH 0

#define CRESCENT_VERSION (CRESCENT_VERSION_MAJOR * 10 + CRESCENT_VERSION_MINOR)
#define CRESCENT_RELEASE (CRESCENT_VERSION * 10 + CRESCENT_VERSION_PATCH)
#define CRESCENT_AUTHORS "mochji"

/*
 * ============================================================================
 * Platform configuration
 *
 * Change these definitions, if needed, to compile Crescent for a specific
 * platform.
 * ============================================================================
 */

#define CRESCENT_32INT (UINT_MAX >> 31)

/*
 * ============================================================================
 * Type configuration
 * ============================================================================
 */

#define CRESCENT_INTEGER_INT   0
#define CRESCENT_INTEGER_LONG  1
#define CRESCENT_INTEGER_LLONG 2

#define CRESCENT_FLOAT_FLOAT   0
#define CRESCENT_FLOAT_DOUBLE  1
#define CRESCENT_FLOAT_LDOUBLE 2

/*
 * @ CRESCENT_32BIT
 *
 * Change this definition to a non-zero value to restrict Crescent to 32-bit
 * number types, rather than the default 64-bit types.
 */

#define CRESCENT_32BIT 0

/*
 * @ CRESCENT_INTEGER_TYPE
 * @ CRESCENT_FLOAT_TYPE
 *
 * Type of Crescent integers and floats.
 */

#if CRESCENT_32BIT
#	if CRESCENT_32INT
#		define CRESCENT_INTEGER_TYPE CRESCENT_INTEGER_INT
#	else
#		define CRESCENT_INTEGER_TYPE CRESCENT_INTEGER_LONG
#	endif
#	define CRESCENT_FLOAT_TYPE       CRESCENT_FLOAT_FLOAT
#else
#	define CRESCENT_INTEGER_TYPE CRESCENT_INTEGER_LLONG
#	define CRESCENT_FLOAT_TYPE   CRESCENT_FLOAT_DOUBLE
#endif

/*
 * @ CRESCENT_STRING_ALLOCSPACE
 * @ CRESCENT_ARRAY_ALLOCSPACE
 *
 * THESE DEFINITIONS WILL BE REMOVED !!
 */

#define CRESCENT_STRING_ALLOCSPACE 32
#define CRESCENT_ARRAY_ALLOCSPACE 32

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

#define CRESCENT_TYPE_NIL       0  /* 0b00000000 */
#define CRESCENT_TYPE_BOOLEAN   8  /* 0b00001000 */
#define CRESCENT_TYPE_INTEGER   1  /* 0b00000001 */
#define CRESCENT_TYPE_FLOAT     9  /* 0b00001001 */
#define CRESCENT_TYPE_STRING    2  /* 0b00000010 */
#define CRESCENT_TYPE_ARRAY     10 /* 0b00001010 */
#define CRESCENT_TYPE_CFUNCTION 4  /* 0b00000100 */

#define CRESCENT_STATUS_OK    0
#define CRESCENT_STATUS_ERROR 1
#define CRESCENT_STATUS_NOMEM 2

#if CRESCENT_INTEGER_TYPE == CRESCENT_INTEGER_INT
#	define CRESCENT_INTEGER     int
#	define CRESCENT_INTEGER_FMT "%d"
#	define CRESCENT_INTEGER_MAX INT_MAX
#	define CRESCENT_INTEGER_MIN INT_MIN
#elif CRESCENT_INTEGER_TYPE == CRESCENT_INTEGER_LONG
#	define CRESCENT_INTEGER     long
#	define CRESCENT_INTEGER_FMT "%ld"
#	define CRESCENT_INTEGER_MAX LONG_MAX
#	define CRESCENT_INTEGER_MIN LONG_MIN
#elif CRESCENT_INTEGER_TYPE == CRESCENT_INTEGER_LLONG
#	define CRESCENT_INTEGER     long long
#	define CRESCENT_INTEGER_FMT "%lld"
#	define CRESCENT_INTEGER_MAX LLONG_MAX
#	define CRESCENT_INTEGER_MIN LLONG_MIN
#endif

#if CRESCENT_FLOAT_TYPE == CRESCENT_FLOAT_FLOAT
#	define CRESCENT_FLOAT     float
#	define CRESCENT_FLOAT_FMT "%f"
#elif CRESCENT_FLOAT_TYPE == CRESCENT_FLOAT_DOUBLE
#	define CRESCENT_FLOAT     double
#	define CRESCENT_FLOAT_FMT "%lf"
#elif CRESCENT_FLOAT_TYPE == CRESCENT_FLOAT_LDOUBLE
#	define CRESCENT_FLOAT     long double
#	define CRESCENT_FLOAT_FMT "%llf"
#endif

struct crescent_State;

typedef CRESCENT_INTEGER crescent_Integer;
typedef CRESCENT_FLOAT   crescent_Float;
typedef int             (crescent_CFunction)(struct crescent_State*);

#endif
