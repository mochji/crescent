/*
 * https://github.com/mochji/crescent
 * types/function.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_CORE_FUNCTION_H
#define CRS_CORE_FUNCTION_H

#include "crescent/conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/buffer.h"

crs_Function* crsK_new(crs_Thread* thread, unsigned nI, unsigned nC,
                                           unsigned nN, int debug);
void          crsK_free(crs_Thread* thread, crs_Function* func);
int           crsK_dump(crs_Dump* dump, crs_Function* func);
crs_Function* crsK_load(crs_Stream* stream);

#endif
