/*
 * https://github.com/mochji/crescent
 * core/debug.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_CORE_DEBUG_H
#define CRS_CORE_DEBUG_H

#include "crescent/conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/buffer.h"

int         crsD_getLine(crs_Function* func, unsigned pc);
crs_String* crsD_loadError(crs_Thread* thread, crs_Stream* stream);

#endif
