/*
 * https://github.com/mochji/crescent
 * core/debug.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include "crescent/conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/state.h"
#include "core/format.h"

#include "core/debug.h"

crs_String* crsD_addInfo(crs_Thread* thread, char* source, int line) {
    char* error = crsO_toString(&thread->error, NULL);
    error       = error != NULL ? error : "fix this!";

    if (line) {
        return crsF_format(thread, "%s:%d: %s", source, line, error);
    } else {
        return crsF_format(thread, "%s: %s", source, error);
    }
}
