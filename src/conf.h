#ifndef CONF_H
#define CONF_H

#include <stdint.h>

// #define CRESCENT_BITNESS 32

#if   SIZE_MAX == 0xFFFFFFFF         && !defined(CRESCENT_BITNESS)
	#define CRESCENT_BITNESS 32
#elif SIZE_MAX == 0xFFFFFFFFFFFFFFFF && !defined(CRESCENT_BITNESS)
	#define CRESCENT_BITNESS 64
#elif !defined(CRESCENT_BITNESS)
	#error "Platform is not 32 or 64 bit, manually define CRESCENT_BITNESS if this is a mistake."
#endif

#if CRESCENT_BITNESS == 32
	#define CRESCENT_INTEGER int32_t
	#define CRESCENT_FLOAT   float
#else
	#define CRESCENT_INTEGER int64_t
	#define CRESCENT_FLOAT   double
#endif

#endif
