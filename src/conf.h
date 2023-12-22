/*
	conf.h

	See license in crescent.c
*/

#ifndef CONF_H
#define CONF_H

#include <stdint.h>

/*
	Configuration file for Crescent, contains definitions that affect both
	compile time and runtime.

	Search for '@' to look for all configurable definitions.

	Search for '@C' to look for all configurable compile time definitions.
	Search for '@R' to look for all configurable runtime definitions.
*/

/*
	==================== Platform configuration ====================

	Definitions that control compilation on different platforms. Definitions
	that control Stuff like bitness, types, 
*/

/*
	@C CRESCENT_BITNESS

	Manually uncomment and define this if the detection for the bitness fails or
	if you want to compile for a different bitness.
*/

// #define CRESCENT_BITNESS 32

#if   SIZE_MAX == 0xFFFF
	#define CRESCENT_BITNESS 16
#elif SIZE_MAX == 0xFFFFFFFF
	#define CRESCENT_BITNESS 32
#elif SIZE_MAX == 0xFFFFFFFFFFFFFFFF
	#define CRESCENT_BITNESS 64
#else
	#error "Platform is not 16, 32 or 64 bit.\nManually define CRESCENT_BITNESS if this is a mistake."
#endif

#if CRESCENT_BITNESS == 16
	#warning "Compiling for 16 bits, most code written in Crescent may not support 16 bit!"
#endif

/*
	==================== Stack configuration ====================

	Definitions that control the stack's behavior in Crescent.
*/

/*
	@R CRESCENT_STRING_MAX_FREE

	Controls the maximum amount of free characters in a string. If the amount of
	characters in the string is greater than this, then reallocate the string
	so the amount of free characters is amount of minimum free characters.
*/

#define CRESCENT_STRING_MAX_FREE 256

/*
	@R CRESCENT_STRING_MIN_FREE

	Controls the minimum amount of free characters in a string. If the amount of
	characters in the string is less than this, then reallocate the string
	so the amount of free characters is amount of maximum free characters.
*/

#define CRESCENT_STRING_MIN_FREE 8

/*
	==================== Stack configuration ====================

	Definitions that control string behavior in Crescent, both in the C API and
	inside Crescent itself.
*/

/*
	@R CRESCENT_STACK_MAX_FREE

	Controls the maximum amount of free items in a stack. If the amount of items
	in the stack is greater than this, then reallocate the stack so the amount
	of free items is amount of minimum free items.
*/

#define CRESCENT_STACK_MAX_FREE 256

/*
	@R CRESCENT_STACK_MIN_FREE

	Controls the minimum amount of free items in a stack. If the amount of items
	in the stack is less than this, then reallocate the stack so the amount of
	free items is amount of maximum free items.
*/

#define CRESCENT_STACK_MIN_FREE 8

#endif
