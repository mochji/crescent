/*
 * https://github.com/mochji/crescent
 * vm/vm.h
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * MIT License
 */

#ifndef VM_VM_H
#define VM_VM_H

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

extern int
crescentV_compare(crescent_Object* a, crescent_Object* b);

extern int
crescentV_call(crescent_State* state, crescent_Object* object, int argCount, int maxResults);

extern int
crescentV_pCall(crescent_State* state, crescent_Object* object, int argCount, int maxResults, int* status);

#endif
