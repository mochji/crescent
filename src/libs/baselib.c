/*
 * https://github.com/mochji/crescent
 * libs/baselib.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdio.h>
#include <stddef.h>

#include "crescent/api.h"
#include "crescent/aux.h"

static int base_error(crs_Thread* thread) {
    crs_error(thread, 1);
    return 0;
}

static void base_warnF(char* msg, void* data) {
    (void)data;
    printf("warning: %s\n", msg);
}

int base_openLib(crs_Thread* thread) {
    crsX_Func lib[] = {
        {"error", &base_error},
        {NULL, NULL}
    };

    crs_setWarnF(thread, &base_warnF, NULL);

    crsX_lib(thread, lib);
    crs_pushString(thread, CRS_VERSION_STR);
    crsX_setG(thread, "_G", CRS_GLOBALS);
    crsX_setG(thread, "_VERSION", -1);
    crs_pop(thread, 1);

    return 0;
}
