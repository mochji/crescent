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

void
crsM_error(crs_Thread* thread);

void*
crsM_malloc(crs_Thread* thread, size_t size);

void*
crsM_realloc(crs_Thread* thread, void* block, size_t size, size_t oldSize);

void
crsM_free(crs_Thread* thread, void* block, size_t size);

#define mem_new(t, o)  (crsM_malloc((t), sizeof(o)));
#define mem_free(t, o) (crsM_free((t), (o), sizeof(*(o))));

#define mem_alloc(t, s)         (crsM_malloc((t), (s)));
#define mem_realloc(t, b, s, p) (crsM_realloc((t), (b), (s), (p)))
#define mem_dealloc(t, b, s)    (crsM_free((t), (b), (s)));

#define mem_vnew(t, n, o) (crsM_malloc((t), (n) * sizeof(o)))
#define mem_vresize(t, v, n, p) \
	(crsM_realloc((t), (v), (n) * sizeof(*(v)), (p) * sizeof(*(v))))
#define mem_vfree(t, v, n) (crsM_free((t), (v), (n) * sizeof(*(v))))

#endif
