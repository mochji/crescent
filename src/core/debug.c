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
#include "core/buffer.h"

#include "core/debug.h"

crs_String* crsD_loadError(crs_Thread* thread, crs_Stream* stream) {
    char* error;

    if (!crsO_toString(&thread->error, &error)) {
        error = "(error is not a string)";
    }

    if (stream->line) {
        return crsF_format(thread, "%s:%d: %s",
            stream->source, stream->line, error);
    } else {
        return crsF_format(thread, "%s: %s", stream->source, error);
    }
}
