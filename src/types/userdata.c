/*
 * https://github.com/mochji/crescent
 * types/userdata.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>
#include <stdint.h>

#include "crescent/conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/memory.h"
#include "core/call.h"
#include "core/gc.h"

#include "types/userdata.h"

crs_UData* crsU_new(crs_Thread* thread, size_t size) {
    if (size > SIZE_MAX - sizeof(crs_UData)) {
        crsC_error(thread, "userdata too large");
    }

    crs_UData* udata = mem_alloc(thread, sizeof(crs_UData) + size);

    if (udata == NULL) {
        crsM_error(thread);
    }

    udata->size = size;
    udata->mt   = NULL;
    return crsG_add(thread, udata, CRS_TYPE_USERDATA);
}

void crsU_free(crs_Thread* thread, crs_UData* udata) {
    mem_dealloc(thread, udata, sizeof(crs_UData) + udata->size);
}
