/*
 * https://github.com/mochji/crescent
 * api/aux.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>

#include "crescent/api.h"
#include "crescent/aux.h"

CRS_EXPORT void crsX_getG(crs_Thread* thread, char* name) {
    crs_pushString(thread, name);
    crs_get(thread, CRS_GLOBALS, -1);
    crs_remove(thread, -2);
}

CRS_EXPORT void crsX_setG(crs_Thread* thread, char* name, int index) {
    crs_pushString(thread, name);
    crs_set(thread, CRS_GLOBALS, -1, index < 0 ? index - 1 : index);
    crs_pop(thread, 1);
}

CRS_EXPORT void crsX_lib(crs_Thread* thread, crsX_Func* func) {
    while (func->name != NULL) {
        crs_pushString(thread, func->name);
        crs_pushCFunction(thread, func->func);
        crs_set(thread, CRS_GLOBALS, -2, -1);
        crs_pop(thread, 2);
        func++;
    }
}
