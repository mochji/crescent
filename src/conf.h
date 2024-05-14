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
 * Crescent main configuration file
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
 * These definitions contain stuff such as Crescent version, copyright
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
 * @ CRESCENT_BITNESS
 *
 * Un-comment and change this only if the automatic bitness detection fails.
 */

/* #define CRESCENT_BITNESS 32 */

/*
 * ============================================================================
 * Simple type configuration
 *
 * Definitions that control the types of simple types in Crescent.
 * ============================================================================
 */

/*
 * @ CRESCENT_INTEGER32
 * @ CRESCENT_FLOAT32
 *
 * Types of crescent_Integer and crescent_Float respectively on a 32-bit
 * platform.
 */

#define CRESCENT_INTEGER32 int32_t
#define CRESCENT_FLOAT32   float

/*
 * @ CRESCENT_INTEGER64
 * @ CRESCENT_FLOAT64
 *
 * Types of crescent_Integer and crescent_Float respectively on a 64-bit
 * platform.
 */

#define CRESCENT_INTEGER64 int64_t
#define CRESCENT_FLOAT64   double

/*
 * ============================================================================
 * Crescent stack configuration
 *
 * Definitions that control the behavior of the Crescent stack, mainly memory
 * allocation.
 * ============================================================================
 */

/*
 * @ CRESCENT_STACK_INITSIZE
 *
 * Controls the initial size of the Crescent stack when first allocated.
 */

#define CRESCENT_STACK_INITSIZE 64

/*
 * @ CRESCENT_STACK_GROWTHRESHOLD
 * @ CRESCENT_STACK_SHRINKTHRESHOLD
 *
 * Controls the percentage of the top to size ratio of the Crescent stack must
 * exceed when growing or shrinking respectively.
 *
 * These 2 values should be at least CRESCENT_STACK_SHRINKTHRESHOLD apart,
 * since resizing multiplies or divides the stack size by 2, doing the
 * opposite for the stack usage. If not, it will resize but a stack
 * operation may cause it to resize again and again, being very slow.
 */

#define CRESCENT_STACK_GROWTHRESHOLD 90
#define CRESCENT_STACK_SHRINKTHRESHOLD 40

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
 * Controls how much is added to the length when allocating or reallocating to
 * get the total allocated size for the string.
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
 * Controls how much is added to the length when allocating or reallocating to
 * get the total allocated size for the array.
 */

#define CRESCENT_ARRAY_ALLOCSPACE 32

/*
 * ============================================================================
 * Crescent VM configuration
 *
 * Definitions that control the behavior of the Crescent virtual machine.
 * ============================================================================
 */

/*
 * @ CRESCENT_VM_MAXLOCALS
 *
 * Controls the max amount of locals in the Crescent VM, must be less than or
 * equal to 200.
 *
 * You can't have unlimited locals/stack indexes since the operands passed to
 * functions that take stack indexes are 8 bit, so only 256 total stack indexes
 * are possible. It's not 256 since you need extra to pass arguments to
 * functions and do other stuff.
 */

#define CRESCENT_VM_MAXLOCALS 200

/*
 * ============================================================================
 * End of configurable definitions
 * ============================================================================
 */

#ifdef __cplusplus
#	error Crescent is not supported for C++. (this is a c project, why did you compile this with a c++ compiler?)
#endif

#ifndef CRESCENT_BITNESS
#	if   SIZE_MAX == 0xFFFFFFFF
#		define CRESCENT_BITNESS 32
#	elif SIZE_MAX == 0xFFFFFFFFFFFFFFFF
#		define CRESCENT_BITNESS 64
#	else
#		error Crescent is only supported for 32-bit and 64-bit platforms. Manually un-comment and define 'CRESCENT_BITNESS' in conf.h if this is an error.
#	endif
#endif

#if   CRESCENT_BITNESS == 32
#	define CRESCENT_INTEGER CRESCENT_INTEGER32
#	define CRESCENT_FLOAT   CRESCENT_FLOAT32
#elif CRESCENT_BITNESS == 64
#	define CRESCENT_INTEGER CRESCENT_INTEGER64
#	define CRESCENT_FLOAT   CRESCENT_FLOAT64
#else
#	error Crescent is only supported for 32-bit and 64-bit platforms.
#endif

#if CRESCENT_VM_MAXLOCALS > 200
#	undef  CRESCENT_VM_MAXLOCALS
#	define CRESCENT_VM_MAXLOCALS 200
#endif

#define CRESCENT_STATUS_OK     0
#define CRESCENT_STATUS_ERROR  1
#define CRESCENT_STATUS_ERRERR 2
#define CRESCENT_STATUS_ERRMEM 3

#define CRESCENT_TYPE_NIL       0
#define CRESCENT_TYPE_BOOLEAN   1
#define CRESCENT_TYPE_INTEGER   2
#define CRESCENT_TYPE_FLOAT     3
#define CRESCENT_TYPE_STRING    4
#define CRESCENT_TYPE_ARRAY     5
#define CRESCENT_TYPE_CFUNCTION 6

struct crescent_State;

typedef CRESCENT_INTEGER crescent_Integer;
typedef CRESCENT_FLOAT   crescent_Float;
typedef int             (crescent_CFunction)(struct crescent_State*);

#endif
