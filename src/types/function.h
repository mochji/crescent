/*
 * https://github.com/mochji/crescent
 * types/function.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_CORE_FUNCTION_H
#define CRS_CORE_FUNCTION_H

#include "conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/buffer.h"

crs_Function*
crsK_new(crs_Thread* thread, crs_u32 nCode, crs_u32 nConstants, crs_u32 nNested);

void
crsK_free(crs_Thread* thread, crs_Function* function);

#endif
