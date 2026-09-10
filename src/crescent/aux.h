/*
 * https://github.com/mochji/crescent
 * crescent/aux.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_CRESCENT_AUX_H
#define CRS_CRESCENT_AUX_H

#include "crescent/api.h"

typedef struct {
    char*          name;
    crs_CFunction* func;
} crsX_Func;

void crsX_getG(crs_Thread* thread, char* name);
void crsX_setG(crs_Thread* thread, char* name, int index);

void crsX_lib(crs_Thread* thread, crsX_Func* func);

#endif
