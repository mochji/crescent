/*
 * https://github.com/mochji/crescent
 * core/memory.h
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#ifndef CRS_CORE_MEMORY_H
#define CRS_CORE_MEMORY_H

#include "conf.h"
#include "limit.h"

#include "core/state.h"

extern void*
crsM_malloc_(crs_Thread* thread, size_t size);

extern void*
crsM_realloc_(crs_Thread* thread, void* block, size_t size, size_t oldSize);

extern void
crsM_free_(crs_Thread* thread, void* block, size_t size);

#endif
