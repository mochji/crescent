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
#define STR(x) STR_HELPER(x)

#define CRESCENT_VERSION_STR STR(CRESCENT_VERSION_MAJOR) "." STR(CRESCENT_VERSION_MINOR)
#define CRESCENT_RELEASE_STR CRESCENT_VERSION_STR "." STR(CRESCENT_VERSION_PATCH)

#define CRESCENT_AUTHORS   "mochji"
#define CRESCENT_COPYRIGHT CRESCENT_RELEASE_STR "  Copyright (C) 2024  " CRESCENT_AUTHORS

/*
 * ============================================================================
 * Platform configuration
 *
 * Change these definitions, if needed, to compile Crescent for a particular
 * platform.
 * ============================================================================
 */

/*
 * @ CRESCENT_CONF_BITNESS
 *
 * Un-comment and change this only if the automatic bitness detection fails
 */

/* #define CRESCENT_CONF_BITNESS 64 */

/*
 * ============================================================================
 * Primative type configuration
 *
 * Definitions that control the types of primative types in Crescent.
 * ============================================================================
 */

/*
 * @ CRESCENT_CONF_INTEGER32
 * @ CRESCENT_CONF_FLOAT32
 *
 * Types of crescent_Integer and crescent_Float respectively on a 32-bit
 * platform.
 */

#define CRESCENT_CONF_INTEGER32 int32_t
#define CRESCENT_CONF_FLOAT32   float

/*
 * @ CRESCENT_CONF_INTEGER64
 * @ CRESCENT_CONF_FLOAT64
 *
 * Types of crescent_Integer and crescent_Float respectively on a 64-bit
 * platform.
 */

#define CRESCENT_CONF_INTEGER64 int64_t
#define CRESCENT_CONF_FLOAT64   double

/*
 * ============================================================================
 * Crescent stack configuration
 *
 * Definitions that control the behavior of the Crescent stack, mainly memory
 * allocation.
 * ============================================================================
 */

/*
 * @ CRESCENT_CONF_STACK_INITSIZE
 *
 * Controls the initial size of the Crescent stack when first allocated.
 */

#define CRESCENT_CONF_STACK_INITSIZE 64

/*
 * @ CRESCENT_CONF_STACK_GROWTHRESHOLD
 * @ CRESCENT_CONF_STACK_SHRINKTHRESHOLD
 *
 * Controls the percentage of the top to size ratio of the Crescent stack must
 * exceed when growing or shrinking respectively.
 */

#define CRESCENT_CONF_STACK_GROWTHRESHOLD 80
#define CRESCENT_CONF_STACK_SHRINKTHRESHOLD 40

/*
 * ============================================================================
 * End of configurable definitions
 * ============================================================================
 */





































































#ifndef CRESCENT_CONF_BITNESS
#	if   SIZE_MAX == 0xFFFFFFFF
#		define CRESCENT_CONF_BITNESS 32
#	elif SIZE_MAX == 0xFFFFFFFFFFFFFFFF
#		define CRESCENT_CONF_BITNESS 64
#	else
#		error Crescent is only supported for 32-bit and 64-bit platforms. Manually un-comment and define 'CRESCENT_PLATFORM_BITNESS' in conf.h if this is an error.
#	endif
#endif

#if   CRESCENT_CONF_BITNESS == 32
#	define CRESCENT_CONF_INTEGER   CRESCENT_CONF_INTEGER32
#	define CRESCENT_CONF_FLOAT     CRESCENT_CONF_FLOAT32
#	define CRESCENT_CONF_STACK_MAX CRESCENT_CONF_STACK_MAX32
#elif CRESCENT_CONF_BITNESS == 64
#	define CRESCENT_CONF_INTEGER CRESCENT_CONF_INTEGER64
#	define CRESCENT_CONF_FLOAT   CRESCENT_CONF_FLOAT64
#	define CRESCENT_CONF_STACK_MAX CRESCENT_CONF_STACK_MAX64
#else
#	error Crescent is only supported for 32-bit and 64-bit platforms.
#endif

#endif
