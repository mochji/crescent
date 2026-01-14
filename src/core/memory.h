/*
 * https://github.com/mochji/crescent
 * core/memory.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_CORE_MEMORY_H
#define CRS_CORE_MEMORY_H

#include <stddef.h>

#include "conf.h"
#include "limit.h"

#include "core/state.h"

extern void*
crsM_malloc_(crs_Thread* thread, size_t size);

extern void*
crsM_realloc_(crs_Thread* thread, void* block, size_t size, size_t oldSize);

extern void
crsM_free_(crs_Thread* thread, void* block, size_t size);

#define mem_new(t, s)     (crsM_malloc_((t), (s)))
#define mem_free(t, b, s) (crsM_free_((t), (b), (s)))

#define mem_vnew(t, n, o)          (crsM_malloc_((t), (n) * sizeof(o)))
#define mem_vresize(t, v, n, p, o) (crsM_realloc_((t), (v), (n) * sizeof(o), (p) * sizeof(o)))
#define mem_vfree(t, v, n, o)      (crsM_free_((t), (v), (n) * sizeof(o)))

#define mem_snew(t, n)          (crsM_malloc_((t), (n) * sizeof(char)))
#define mem_sresize(t, s, n, p) (crsM_realloc_((t), (s), (n) * sizeof(char), (p) * sizeof(char)))
#define mem_sfree(t, s, n)      (crsM_free_((t), (s), (n) * sizeof(char)))

#endif
