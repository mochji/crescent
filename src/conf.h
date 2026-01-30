/*
 * https://github.com/mochji/crescent
 * conf.h
 *
 * Copyright (C) 2026 mochji
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

#define CRS_TYPE_NIL       0  /* 0000 0000 */
#define CRS_TYPE_BOOLEAN   16 /* 0001 0000 */
#define CRS_TYPE_INTEGER   1  /* 0000 0001 */
#define CRS_TYPE_FLOAT     17 /* 0001 0001 */
#define CRS_TYPE_CFUNCTION 32 /* 0010 0000 */
#define CRS_TYPE_STRING    2  /* 0000 0010 */
#define CRS_TYPE_ARRAY     6  /* 0000 0110 */
#define CRS_TYPE_THREAD    14 /* 0000 1110 */

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

struct crs_Thread;
typedef struct crs_Thread crs_Thread;

typedef CRS_INTEGER crs_Integer;
typedef CRS_FLOAT   crs_Float;
typedef int        (crs_CFunction)(struct crs_Thread*);

#endif
