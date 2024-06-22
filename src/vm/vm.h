/*
 * https://github.com/mochji/crescent
 * vm/vm.h
 *
 * Copyright (C) 2024 mochji
 * MIT License
 */

#ifndef CRESCENT_VM_VM_H
#define CRESCENT_VM_VM_H

#include <stddef.h>

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

extern int
crescentV_compare(crescent_Object* a, crescent_Object* b);

extern size_t
crescentV_length(crescent_State* state, crescent_Object* object);

extern int
crescentV_call(crescent_State* state, crescent_Object* object, int args, int maxResults);

extern int
crescentV_pCall(crescent_State* state, crescent_Object* object, int args, int maxResults, int* status);

#endif
