/*
 * https://github.com/mochji/crescent
 * api/std.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>

#include "crescent/api.h"
#include "crescent/aux.h"
#include "crescent/std.h"

int base_openLib(crs_Thread* thread);

CRS_EXPORT void crsX_stdLibs(crs_Thread* thread, unsigned selected) {
    crs_CFunction* libs[] = {
        &base_openLib
    };

    for (unsigned i = 0; i < sizeof(libs) / sizeof(crs_CFunction*); i++) {
        if (selected & (1 << i)) {
            libs[i](thread);
            crs_setTop(thread, 0);
        }
    }
}
