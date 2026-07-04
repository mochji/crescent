/*
 * https://github.com/mochji/crescent
 * types/function.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <string.h>

#include "conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/memory.h"
#include "core/buffer.h"

#include "types/function.h"

void
crsK_free(crs_Thread* thread, crs_Function* function) {
	mem_vfree(thread, function->code, function->nCode);
	mem_vfree(thread, function->constants, function->nConstants);
	mem_free(thread, function);
}
