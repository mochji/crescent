#ifndef PREFIX_H
#define PREFIX_H

#include <stdint.h>

#if UINTPTR_MAX == 0xFFFFFFFF
	#define CRESCENT_BITNESS 32
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
	#define CRESCENT_BITNESS 64
#else
	#error "bitness unknown"
#endif

#define crescent_apiError(msg) fprintf(stderr, "internal error!\n\n%s\n\tin internal function %s\n", msg, __func__); exit(EXIT_FAILURE);

#define crescent_bitCast(type, x) *(type*)&(x)

#endif
