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
#include "core/state.h"
#include "core/buffer.h"

char* crsD_source(crs_Frame* frame, int* what);
int   crsD_params(crs_Frame* frame);
int   crsD_line(crs_Frame* frame);
void  crsD_func(crs_Frame* frame, crs_Object* object);

crs_String* crsD_loadError(crs_Thread* thread, crs_Stream* stream);

#endif
