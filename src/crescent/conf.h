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

#define CRS_STRHELPER(x) #x
#define CRS_STR(x)       CRS_STRHELPER(x)

#define CRS_VERSION_MAJOR 0
#define CRS_VERSION_MINOR 1
#define CRS_VERSION_PATCH 0
#define CRS_AUTHORS       "mochji"

#define CRS_VERSION_STR \
    "Crescent " CRS_STR(CRS_VERSION_MAJOR) "." CRS_STR(CRS_VERSION_MINOR)
#define CRS_RELEASE_STR \
    CRS_VERSION_STR "." CRS_STR(CRS_VERSION_PATCH)

#define CRS_VERSION (CRS_VERSION_MAJOR * 10 + CRS_VERSION_MINOR)
#define CRS_RELEASE (CRS_VERSION * 10 + CRS_VERSION_PATCH)

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
 * =============================================================================
 * Language Configuration
 *
 * The following definitions can be uncommented to enable or disable optional
 * language features.
 * =============================================================================
 */

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

#define CRS_TNIL      0
#define CRS_TBOOLEAN  1
#define CRS_TNUMBER   2
#define CRS_TSTRING   3
#define CRS_TTABLE    4
#define CRS_TFUNCTION 5
#define CRS_TTHREAD   6
#define CRS_TUSERDATA 7

#define CRS_TYPECOUNT 8

#define CRS_OK      0 /* no error               */
#define CRS_ERROR   1 /* runtime error          */
#define CRS_CODEERR 2 /* load/compilation error */
#define CRS_MEMERR  3 /* out of memory          */

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

#define CRS_RETALL (-1)

struct crs_Thread;
typedef struct crs_Thread crs_Thread;
typedef int  (crs_Reader)(crs_Thread*, void*, char*, int*);
typedef int  (crs_Writer)(crs_Thread*, void*, char*, int);
typedef void (crs_WarnFunc)(char*, void*);

typedef CRS_INTEGER  crs_Integer;
typedef CRS_UNSIGNED crs_Unsigned;
typedef CRS_FLOAT    crs_Float;
typedef CRS_DAPFLOAT crs_DAPFloat;
typedef int         (crs_CFunction)(struct crs_Thread*);

typedef struct crs_Debug {
    char* source;
    char* name;
    int   what; /* function type */
    int   from; /* called from */
    int   params;
    int   line;
} crs_Debug;

#define CRS_WHAT_C    0
#define CRS_WHAT_VM   1
#define CRS_WHAT_MAIN 2

#define CRS_FROM_UNKNOWN 0
#define CRS_FROM_LOCAL   1
#define CRS_FROM_GLOBAL  2
#define CRS_FROM_MM      3

#define CRS_NORET  __attribute__((noreturn))
#define CRS_EXPORT __attribute__((visibility("default")))

#endif
