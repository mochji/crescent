/*
 * conf.h
 *
 * Crescent configuration file
 *
 * This file contains definitions that control both compile time and runtime.
 * Such definitions include types, allocation sizes, etc.
 *
 * Search for '@' to find all configurable definitions.
 *
 * Search for '@C' to find all configurable compile time definitions.
 * Search for '@R' to find all configurable runtime definitions.
 */

#ifndef CONF_H
#define CONF_H

#include <stdint.h>

/*
 * @C CRESCENT_BITNESS
 *
 * Manually un-comment and define this if the automatic bitness check fails to
 * correctly detect your platform's bitness.
 */

/* #define CRESCENT_BITNESS 32 */

/*
 * @C CRESCENT_TYPE_INT32
 *
 * Controls the type used for integers when compiled on a 32 bit platform.
 */

#define CRESCENT_TYPE_INT32 int32_t

/*
 * @C CRESCENT_TYPE_FLOAT32
 *
 * Controls the type used for floats when compiled on a 32 bit platform.
 */

#define CRESCENT_TYPE_FLOAT32 float

/* #define CRESCENT_BITNESS 64 */

/*
 * @C CRESCENT_TYPE_INT64
 *
 * Controls the type used for integers when compiled on a 64 bit platform.
 */

#define CRESCENT_TYPE_INT64 int64_t

/*
 * @C CRESCENT_TYPE_FLOAT64
 *
 * Controls the type used for floats when compiled on a 64 bit platform.
 */

#define CRESCENT_TYPE_FLOAT64 double

/*
 * @R CRESCENT_STACK_GROWTHFACTOR
 *
 * Controls the value that the Crescent stack will multiply the current size
 * by when reallocating memory.
 */

#define CRESCENT_STACK_GROWTHFACTOR 2

/*
 * @R CRESCENT_STACK_MAXFREE
 *
 * Controls the maximum amount of free items in the Crescent stack before being
 * reallocated to free memory.
 */

#define CRESCENT_STACK_MAXFREE 128

/*
 * @R CRESCENT_STACK_MINFREE
 *
 * Controls the minimum amount of free items in the Crescent stack before being
 * reallocated for more memory.
 */

#define CRESCENT_STACK_MINFREE 16

/*
 * ==================================
 * End of configurable definitions
 * ==================================
 */

#if   SIZE_MAX == 0xFFFFFFFF         && !defined(CRESCENT_BITNESS)
	#define CRESCENT_BITNESS 32
	#define CRESCENT_INTEGER CRESCENT_TYPE_INT32
	#define CRESCENT_FLOAT   CRESCENT_TYPE_FLOAT32
#elif SIZE_MAX == 0xFFFFFFFFFFFFFFFF && !defined(CRESCENT_BITNESS)
	#define CRESCENT_BITNESS 64
	#define CRESCENT_INTEGER CRESCENT_TYPE_FLOAT32
	#define CRESCENT_FLOAT   CRESCENT_TYPE_FLOAT64
#elif !defined(CRESCENT_BITNESS)
	#error "Platform is not 32 or 64 bit, manually define CRESCENT_BITNESS if this is a mistake."
#endif

#endif
