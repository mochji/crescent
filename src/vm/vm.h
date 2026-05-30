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

crs_Integer
crsV_length(crs_Thread* thread, crs_Object* object);

crs_Object*
crsV_get(crs_Thread* thread, crs_Object* object, crs_Object* key);

void
crsV_set(crs_Thread* thread, crs_Object* object, crs_Object* key, crs_Object* value);

int
crsV_call(crs_Thread* thread, crs_Object* object, int args, int maxResults);

int
crsV_pCall(crs_Thread* thread, crs_Object* object, int args, int maxResults, int* status);

#endif
