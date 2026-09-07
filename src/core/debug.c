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

int crsD_getLine(crs_Function* func, unsigned pc) {
    Debug_Info* debug = &func->debug;
    Debug_Line* info  = debug->lines;
    unsigned    low   = 0;
    unsigned    high  = debug->nL - 1;

    while (low < high) {
        unsigned mid = low + (high - low) / 2;
        info         = &debug->lines[mid];

        if (info->pc > pc) {
            high = mid - 1;
        } else if ((info + 1)->pc <= pc) {
            low = mid;
        } else { /* info->pc <= pc < (info + 1)->pc */
            break;
        }
    }

    return info->line;
}

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
