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
 * Change these definitions, if needed, to compile Crescent for a particular
 * platform.
 * ============================================================================
 */

/*
 * @ CRESCENT_32BIT
 *
 * Change this definition to a non-zero value to restrict Crescent to 32-bit
 * number types. (default uses 64-bit types)
 */

#define CRESCENT_32BIT 0

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
#	error Crescent is not supported for C++. (not yet)
#endif

/* cannot use sizeof within a preprocessor macro :( */

#define CRESCENT_32INT ((UINT_MAX >> 30) >= 3)

#if CRESCENT_32BIT
#	if CRESCENT_32INT
#		define CRESCENT_INTEGER int
#	else
#		define CRESCENT_INTEGER long
#	endif
#	define CRESCENT_FLOAT float
#else
#	define CRESCENT_INTEGER long long
#	define CRESCENT_FLOAT   double
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
