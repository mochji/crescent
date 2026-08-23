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

crs_String* crsD_addInfo(crs_Thread* thread, char* source, int line);

#endif
