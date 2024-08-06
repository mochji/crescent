/*
 * https://github.com/mochji/crescent
 * limit.h
 *
 * Copyright (C) 2024 mochji
 * MIT License
 */

#ifndef CRESCENT_LIMIT_H
#define CRESCENT_LIMIT_H

#include <stddef.h>
#include <stdint.h>

#include "conf.h"

#define CRESCENT_MAX_SIZET   SIZE_MAX
#define CRESCENT_MAX_INTEGER CRESCENT_INTEGER_MAX

#if CRESCENT_MAX_SIZET > CRESCENT_MAX_INTEGER
#	define CRESCENT_MAX_SIZE CRESCENT_MAX_INTEGER
#else
#	define CRESCENT_MAX_SIZE CRESCENT_MAX_SIZET
#endif

/* global to entire stack */
#define CRESCENT_MIN_STACK 64
#define CRESCENT_MAX_STACK SIZE_MAX

/* local to a stack frame */
#define CRESCENT_MIN_TOP 16
#define CRESCENT_MAX_TOP INT_MAX

#define CRESCENT_MAX_CALLS  4096
#define CRESCENT_MAX_CCALLS 200

#define CRESCENT_MAX_LOCALS 200

#endif
