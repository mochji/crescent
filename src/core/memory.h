/*
 * https://github.com/mochji/crescent
 * core/memory.h
 *
 * Copyright (C) 2026 mochji
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

#define obj_new(t, o, f)  (crsM_malloc_((t), sizeof(o) + (f)))
#define obj_free(t, o, f) (crsM_free_((t), (o), sizeof(*o) + (f)))

#define vec_new(t, n, o)          (crsM_malloc_((t), (n) * sizeof(o)))
#define vec_resize(t, v, n, p, o) (crsM_realloc_((t), (v), (n) * sizeof(o), (p) * sizeof(o)))
#define vec_free(t, v, n, o)      (crsM_free_((t), (v), (n) * sizeof(o)))

#define str_new(t, n)          (crsM_malloc_((t), (n) * sizeof(char)))
#define str_resize(t, s, n, p) (crsM_realloc_((t), (s), (n) * sizeof(char), (p) * sizeof(char)))
#define str_free(t, s, n)      (crsM_free_((t), (s), (n) * sizeof(char)))

#endif
