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

int crsX_loadStr(crs_Thread* thread, char* str, char* source);
int crsX_loadFile(crs_Thread* thread, char* path, char* source);
int crsX_dumpFile(crs_Thread* thread, int index, char* path);

void crsX_getG(crs_Thread* thread, char* name);
void crsX_setG(crs_Thread* thread, char* name, int index);

void crsX_lib(crs_Thread* thread, int index, crsX_Func* func);

#endif
