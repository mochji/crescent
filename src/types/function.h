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

void
crsK_free(crs_Thread* thread, crs_Function* function);

#endif
