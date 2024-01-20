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
 * Platform configuration
 *
 * Change these definitions, if needed, to compile Crescent for a particular
 * platform.
 * ============================================================================
 */

/*
 * @ CRESCENT_PLATFORM_BITNESS
 *
 * Un-comment and change this only if the automatic bitness detection fails
 */

/* #define CRESCENT_PLATFORM_BITNESS 32 */

/*
 * ============================================================================
 * Primative type configuration
 *
 * Definitions that control the types of primative types in Crescent.
 * ============================================================================
 */

/*
 * @ CRESCENT_TYPE_INTEGER32
 * @ CRESCENT_TYPE_FLOAT32
 *
 * Types of crescent_Integer and crescent_Float respectively on a 32-bit
 * platform.
 */

#define CRESCENT_TYPE_INTEGER32 int32_t
#define CRESCENT_TYPE_FLOAT32   float

/*
 * @ CRESCENT_TYPE_INTEGER64
 * @ CRESCENT_TYPE_FLOAT64
 *
 * Types of crescent_Integer and crescent_Float respectively on a 64-bit
 * platform.
 */

#define CRESCENT_TYPE_INTEGER64 int64_t
#define CRESCENT_TYPE_FLOAT64   double

/*
 * ============================================================================
 * Crescent string configuration
 *
 * Definitions that control the behavior Crescent strings.
 * ============================================================================
 */

/*
 * @ CRESCENT_STRING_MAXFREE
 * @ CRESCENT_STRING_MINFREE
 *
 * Controls the maximum and minimum free items in a Crescent string before
 * being reallocated.
 */

#define CRESCENT_STRING_MAXFREE 64
#define CRESCENT_STRING_MINFREE 16

/*
 * ============================================================================
 * Crescent stack configuration
 *
 * Definitions that control the behavior of the Crescent stack, especially
 * memory allocation.
 * ============================================================================
 */

/*
 * @ CRESCENT_STACK_MAXFREE
 * @ CRESCENT_STACK_MINFREE
 *
 * Controls the maximum and minimum free items on the Crescent stack before
 * being reallocated.
 */

#define CRESCENT_STACK_MAXFREE 64
#define CRESCENT_STACK_MINFREE 16

/*
 * @ CRESCENT_STACK_ALLOCSPACE
 *
 * Controls the amount of space between the maximum / minimum amount of free
 * items on the Crescent stack when reallocated.
 */

#define CRESCENT_STACK_ALLOCSPACE 16

/*
 * ============================================================================
 * End of configurable definitions
 * ============================================================================
 */





































































#ifndef CRESCENT_PLATFORM_BITNESS
#	if   SIZE_MAX == 0xFFFFFFFF
#		define CRESCENT_PLATFORM_BITNESS 32
#	elif SIZE_MAX == 0xFFFFFFFFFFFFFFFF
#		define CRESCENT_PLATFORM_BITNESS 64
#	else
#		error "Crescent is only supported for 32-bit and 64-bit platforms. " \
		"Manually un-comment and define 'CRESCENT_PLATFORM_BITNESS' in conf.h if this is an error."
#	endif
#endif

#if   CRESCENT_PLATFORM_BITNESS == 32
#	define CRESCENT_TYPE_INTEGER CRESCENT_TYPE_INTEGER32
#	define CRESCENT_TYPE_FLOAT   CRESCENT_TYPE_FLOAT32
#elif CRESCENT_PLATFORM_BITNESS == 64
#	define CRESCENT_TYPE_INTEGER CRESCENT_TYPE_INTEGER64
#	define CRESCENT_TYPE_FLOAT   CRESCENT_TYPE_FLOAT64
#else
#	error "Crescent is only supported for 32-bit and 64-bit platforms."
#endif

#define CRESCENT_STACK_INITSIZE (CRESCENT_STACK_MAXFREE - CRESCENT_STACK_ALLOCSPACE)

#endif
