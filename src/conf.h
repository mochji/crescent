/*
 * https://github.com/mochji/crescent
 * conf.h
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * Crescent main configuration file
 *
 * Search '@' for all configurable definitions.
 */

#ifndef CONF_H
#define CONF_H

#include <stdint.h>

/*
 * ============================================================================
 * Crescent configuration
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
#define CRESCENT_COPYRIGHT "Crescent " CRESCENT_RELEASE_STR "  Copyright (C) 2024  " CRESCENT_AUTHORS

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
 * Primative type configuration
 *
 * Definitions that control the types of primative types in Crescent.
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
 */

#define CRESCENT_STACK_GROWTHRESHOLD 80
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
 * Crescent VM configuration
 *
 * Definitions that control the behavior of the Crescent virtual machine.
 * ============================================================================
 */

/*
 * @ CRESCENT_VM_MAXLOCALS
 *
 * Controls the max amount of locals in the Crescent VM, must be less than 250.
 */

#define CRESCENT_VM_MAXLOCALS 250

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

#if CRESCENT_VM_MAXLOCALS > 250
#	undef  CRESCENT_VM_MAXLOCALS
#	define CRESCENT_VM_MAXLOCALS 250
#endif

#define CRESCENT_STATUS_OK    0
#define CRESCENT_STATUS_YIELD 1
#define CRESCENT_STATUS_ERROR 2
#define CRESCENT_STATUS_NOMEM 3

#define CRESCENT_TYPE_NONE      0
#define CRESCENT_TYPE_NIL       1
#define CRESCENT_TYPE_BOOLEAN   2
#define CRESCENT_TYPE_INTEGER   3
#define CRESCENT_TYPE_FLOAT     4
#define CRESCENT_TYPE_STRING    5
#define CRESCENT_TYPE_CFUNCTION 6

struct crescent_State;

typedef int              crescent_Boolean;
typedef CRESCENT_INTEGER crescent_Integer;
typedef CRESCENT_FLOAT   crescent_Float;
typedef int             (crescent_CFunction)(struct crescent_State*);

#endif
