/*
 * https://github.com/mochji/crescent
 * conf.h
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * MIT License
 */

/*
 * Public build configuration for Crescent
 *
 * Search '@' for all configurable definitions.
 */

#ifndef CRESCENT_CONF_H
#define CRESCENT_CONF_H

#include <stdint.h>

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

#define STR_HELPER(x) #x
#define STR(x)        STR_HELPER(x)

#define CRESCENT_VERSION_STR STR(CRESCENT_VERSION_MAJOR) "." STR(CRESCENT_VERSION_MINOR)
#define CRESCENT_RELEASE_STR CRESCENT_VERSION_STR "." STR(CRESCENT_VERSION_PATCH)

#define CRESCENT_AUTHORS   "mochji"
#define CRESCENT_COPYRIGHT "Crescent " CRESCENT_RELEASE_STR "  Copyright (C) 2024 " CRESCENT_AUTHORS

#undef STR_HELPER
#undef STR

/*
 * ============================================================================
 * Platform configuration
 *
 * Change these definitions, if needed, to compile Crescent for a particular
 * platform.
 * ============================================================================
 */

/*
 * @ CRESCENT_32BIT
 *
 * Change this definition to a non-zero value to restrict Crescent to 32-bit
 * number types.
 */

#define CRESCENT_32BIT 0

/*
 * ============================================================================
 * Number type configuration
 *
 * Definitions that control the types of number types within Crescent.
 * ============================================================================
 */

/*
 * @ CRESCENT_INTEGER32
 * @ CRESCENT_FLOAT32
 *
 * Type of crescent_Integer and crescent_Float when 32-bit numbers are enabled.
 */

#define CRESCENT_INTEGER32 int32_t
#define CRESCENT_FLOAT32   float

/*
 * @ CRESCENT_INTEGER64
 * @ CRESCENT_FLOAT64
 *
 * Type of crescent_Integer and crescent_Float when 64-bit numbers are enabled.
 */

#define CRESCENT_INTEGER64 int64_t
#define CRESCENT_FLOAT64   double

/*
 * ============================================================================
 * Crescent stack configuration
 *
 * Definitions that control the behavior of the Crescent stack.
 * ============================================================================
 */

/*
 * @ CRESCENT_STACK_INITSIZE
 *
 * The initial and minimum size of the Crescent stack.
 */

#define CRESCENT_STACK_INITSIZE 64

/*
 * ============================================================================
 * Crescent string configuration
 *
 * Definitions that control the behavior of Crescent strings, mainly memory
 * allocation.
 * ============================================================================
 */

/*
 * @ CRESCENT_STRING_ALLOCSPACE
 *
 * How much is added to the length when allocating or reallocating to get the
 * total allocated size for the string.
 */

#define CRESCENT_STRING_ALLOCSPACE 32

/*
 * ============================================================================
 * Crescent array configuration
 *
 * Definitions that control the behavior of Crescent arrays, mainly memory
 * allocation.
 * ============================================================================
 */

/*
 * @ CRESCENT_ARRAY_ALLOCSPACE
 *
 * How much is added to the length when allocating or reallocating to get the
 * total allocated size for the array.
 */

#define CRESCENT_ARRAY_ALLOCSPACE 32

/*
 * ============================================================================
 * End of configurable definitions
 * ============================================================================
 */

#ifdef __cplusplus
#	error Crescent is not supported for C++. (this is a c project, why did you compile this with a c++ compiler?)
#endif

#if   CRESCENT_32BIT
#	define CRESCENT_INTEGER CRESCENT_INTEGER32
#	define CRESCENT_FLOAT   CRESCENT_FLOAT32
#else
#	define CRESCENT_INTEGER CRESCENT_INTEGER64
#	define CRESCENT_FLOAT   CRESCENT_FLOAT64
#endif

#define CRESCENT_STATUS_OK     0
#define CRESCENT_STATUS_ERROR  1
#define CRESCENT_STATUS_ERRMEM 2

/*
 * The last 3 bits within a type tag encode properties about the type:
 *
 * (1 = yes, 0 = no)
 *
 * - 3rd to last (leftmost) bit: Can you query the length of this type?
 * - 2nd to last (middle) bit:   Can you call this type?
 * - last (rightmost) bit:       Is this type a number?
 */

#define CRESCENT_TYPE_NIL       0  /* 0b00000000 */
#define CRESCENT_TYPE_BOOLEAN   8  /* 0b00001000 */
#define CRESCENT_TYPE_INTEGER   1  /* 0b00000001 */
#define CRESCENT_TYPE_FLOAT     9  /* 0b00001001 */
#define CRESCENT_TYPE_STRING    4  /* 0b00000100 */
#define CRESCENT_TYPE_ARRAY     12 /* 0b00001100 */
#define CRESCENT_TYPE_CFUNCTION 2  /* 0b00000010 */

struct crescent_State;

typedef CRESCENT_INTEGER crescent_Integer;
typedef CRESCENT_FLOAT   crescent_Float;
typedef int             (crescent_CFunction)(struct crescent_State*);

#endif
