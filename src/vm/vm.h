/*
 * https://github.com/mochji/crescent
 * vm/vm.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_VM_VM_H
#define CRS_VM_VM_H

#include <stddef.h>

#include "conf.h"
#include "limit.h"

#include "core/object.h"

int
crsV_equal(crs_Thread* thread, crs_Object* l, crs_Object* r);

int
crsV_less(crs_Thread* thread, crs_Object* l, crs_Object* r);

int
crsV_lessEqual(crs_Thread* thread, crs_Object* l, crs_Object* r);

void
crsV_arith(crs_Thread* thread, crs_Object* o, crs_Object* l, crs_Object* r, int op);

crs_Integer
crsV_length(crs_Thread* thread, crs_Object* object);

crs_Object*
crsV_get(crs_Thread* thread, crs_Object* object, crs_Object* key);

void
crsV_set(crs_Thread* thread, crs_Object* object, crs_Object* key, crs_Object* value);

void
crsV_call(crs_Thread* thread, crs_Object* object, int args, int wanted);

int
crsV_pcall(crs_Thread* thread, crs_Object* object, int args, int wanted);

#endif
