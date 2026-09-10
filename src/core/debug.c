/*
 * https://github.com/mochji/crescent
 * core/debug.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>

#include "crescent/conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/state.h"
#include "core/format.h"
#include "core/buffer.h"

#include "core/debug.h"

char* crsD_source(crs_Frame* frame, int* what) {
    if (!call_isvm(frame)) {
        crs_Function* func = frame->i.v.f;
        *what              = func->flags & FUNC_MAIN
            ? CRS_DBG_MAIN
            : CRS_DBG_VM;

        if (func_hasdebug(func)) {
            return func->debug.source->contents;
        }
    }

    *what = CRS_DBG_C;
    return "?";
}

int crsD_params(crs_Frame* frame) {
    if (call_isvm(frame)) {
        return frame->i.v.f->args;
    }

    return 0;
}

int crsD_line(crs_Frame* frame) {
    if (!call_hasdebug(frame)) {
        return 0;
    }

    crs_Function* func  = frame->i.v.f;
    Debug_Info*   debug = &func->debug;
    Debug_Line*   line  = debug->lines;
    unsigned      low   = 0;
    unsigned      high  = debug->nL - 1;
    unsigned      pc    = (unsigned)(frame->i.v.pc - func->code);

    while (low <= high) {
        unsigned mid = low + (high - low) / 2;
        line         = &debug->lines[mid];

        if (low == high) {
            break; /* found it */
        }

        if (line->pc > pc) {
            high = mid - 1;
        } else if ((line + 1)->pc <= pc) {
            /* line->pc <= pc >= (line + 1)->pc */
            low = mid + 1;
        } else {
            /* line->pc <= pc < (line + 1)->pc */
            break;
        }
    }

    return line->line;
}

void crsD_func(crs_Frame* frame, crs_Object* object) {
    if (call_isvm(frame)) {
        obj_setgc(object, frame->i.v.f);
    }

    obj_setc(object, frame->i.c.f);
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
