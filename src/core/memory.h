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

extern void
crsM_error(crs_Thread* thread);

extern void*
crsM_malloc(crs_Thread* thread, size_t size);

extern void*
crsM_realloc(crs_Thread* thread, void* block, size_t size, size_t oldSize);

extern void
crsM_free(crs_Thread* thread, void* block, size_t size);

#define mem_new(t, o)  (crsM_malloc((t), sizeof(o)));
#define mem_free(t, o) (crsM_free((t), (o), sizeof(o)));

#define mem_alloc(t, s)      (crsM_malloc((t), (s)));
#define mem_dealloc(t, b, s) (crsM_free((t), (b), (s)));

#define mem_vnew(t, n, o)          (crsM_malloc((t), (n) * sizeof(o)))
#define mem_vresize(t, v, n, p, o) (crsM_realloc((t), (v), (n) * sizeof(o), (p) * sizeof(o)))
#define mem_vfree(t, v, n, o)      (crsM_free((t), (v), (n) * sizeof(o)))

#define mem_snew(t, n)          (crsM_malloc((t), ((n) + 1) * sizeof(char)))
#define mem_sresize(t, s, n, p) (crsM_realloc((t), (s), ((n) + 1) * sizeof(char), ((p) + 1) * sizeof(char)))
#define mem_sfree(t, s, n)      (crsM_free((t), (s), ((n) + 1) * sizeof(char)))

#endif
