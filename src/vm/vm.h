/*
 * https://github.com/mochji/crescent
 * vm/vm.h
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#ifndef CRS_VM_VM_H
#define CRS_VM_VM_H

#include <stddef.h>

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

extern int
crsV_compare(crs_Object* a, crs_Object* b);

extern size_t
crsV_length(crs_Thread* thread, crs_Object* object);

extern int
crsV_call(crs_Thread* thread, crs_Object* object, int args, int maxResults);

extern int
crsV_pCall(crs_Thread* thread, crs_Object* object, int args, int maxResults, int* status);

#endif
