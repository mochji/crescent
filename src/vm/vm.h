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

#include "crescent/conf.h"
#include "limit.h"

#include "core/object.h"

int crsV_execute(crs_Thread* thread, crs_Function* function);

#endif
