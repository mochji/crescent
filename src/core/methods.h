/*
 * https://github.com/mochji/crescent
 * core/methods.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_CORE_METHODS_H
#define CRS_CORE_METHODS_H

#include "crescent/conf.h"
#include "limit.h"

int         crsM_equal(crs_Object* l, crs_Object* r);
int         crsM_compare(crs_Thread* thread, crs_Object* l, crs_Object* r,
                                             int op);
crs_Integer crsM_length(crs_Thread* thread, crs_Object* object);
int         crsM_rawArith(crs_Object* o, crs_Object* l, crs_Object* r, int op);
void        crsM_arith(crs_Thread* thread, crs_Object* o, crs_Object* l,
                                           crs_Object* r, int op);
crs_Object* crsM_get(crs_Thread* thread, crs_Object* object, crs_Object* key);
void        crsM_set(crs_Thread* thread, crs_Object* object, crs_Object* key,
                                         crs_Object* value);

void crsM_call(crs_Thread* thread, crs_Object* object, int args, int wanted);
int  crsM_pcall(crs_Thread* thread, crs_Object* object, int args, int wanted);

#endif
