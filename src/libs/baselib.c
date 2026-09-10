/*
 * https://github.com/mochji/crescent
 * libs/baselib.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include "crescent/api.h"
#include "crescent/aux.h"

static int base_error(crs_Thread* thread) {
    crs_error(thread, 1);
    return 0;
}

int base_openLib(crs_Thread* thread) {
    crsX_Func lib[] = {
        {"error", &base_error},
        {NULL, NULL}
    };

    crsX_lib(thread, lib);
    crs_pushString(thread, CRS_VERSION_STR);
    crsX_setG(thread, "_G", CRS_GLOBALS);
    crsX_setG(thread, "_VERSION", -1);
    crs_pop(thread, 1);

    return 0;
}
