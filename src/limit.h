/*
 * https://github.com/mochji/crescent
 * limit.h
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * MIT License
 */

#ifndef CRESCENT_LIMIT_H
#define CRESCENT_LIMIT_H

#include <stddef.h>

#include "conf.h"

#define CRESCENT_MAX_INTEGER (~((crescent_Integer)0))
#define CRESCENT_MAX_SIZET   (~((size_t)0))

#define CRESCENT_MAX_SIZE                        \
	(CRESCENT_MAX_SIZET > CRESCENT_MAX_INTEGER ? \
		CRESCENT_MAX_INTEGER :                   \
		CRESCENT_MAX_SIZET                       \
	)

#define CRESCENT_MAX_CALLS  4096
#define CRESCENT_MAX_CCALLS 200

#define CRESCENT_MAX_LOCALS 200



#endif
