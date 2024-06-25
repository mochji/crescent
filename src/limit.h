/*
 * https://github.com/mochji/crescent
 * limit.h
 *
 * Copyright (C) 2024 mochji
 * MIT License
 */

#ifndef CRESCENT_LIMIT_H
#define CRESCENT_LIMIT_H

#include <limits.h>

#include "conf.h"

#if CRESCENT_32BIT
#	if CRESCENT_32INT
#		define CRESCENT_MAX_INTEGER INT_MAX
#	else
#		define CRESCENT_MAX_INTEGER LONG_MAX
#	endif
#else
#	define CRESCENT_MAX_INTEGER LLONG_MAX
#endif

#define CRESCENT_MAX_SIZET (~((size_t)0))

#define CRESCENT_MAX_SIZE                        \
	(CRESCENT_MAX_SIZET > CRESCENT_MAX_INTEGER ? \
		CRESCENT_MAX_INTEGER :                   \
		CRESCENT_MAX_SIZET                       \
	)

/* TODO: actually use MAX_STACK, MIN_TOP and MAX_TOP */

/* global to entire stack */
#define CRESCENT_MIN_STACK 64
#define CRESCENT_MAX_STACK CRESCENT_MAX_SIZET

/* local to a stack frame */
#define CRESCENT_MIN_TOP 8
#define CRESCENT_MAX_TOP (INT_MAX - 1)

#define CRESCENT_MAX_CALLS  4096
#define CRESCENT_MAX_CCALLS 200

#define CRESCENT_MAX_LOCALS 200

#endif
