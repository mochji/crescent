/*
 * https://github.com/mochji/crescent
 * limit.h
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#ifndef CRS_LIMIT_H
#define CRS_LIMIT_H

#include <stdint.h>

#include "conf.h"

#define CRS_MAX_SIZE           \
	SIZE_MAX > CRS_INTEGER_MAX \
		? CRS_INTEGER_MAX      \
		: SIZE_MAX

/* global to entire stack */
#define CRS_MIN_STACK 64
#define CRS_MAX_STACK SIZE_MAX

/* local to a stack frame */
#define CRS_MIN_TOP 16
#define CRS_MAX_TOP INT_MAX

#define CRS_MAX_CALLS  4096
#define CRS_MAX_CCALLS 200

#define CRS_MAX_LOCALS 200

#endif
