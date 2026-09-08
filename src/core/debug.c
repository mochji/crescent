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

char* crsD_getSource(crs_Frame* frame, int* what) {
    if (frame->flags & CALL_VM) {
        crs_Function* func = frame->i.v.f;
        *what              = func->flags & FUNC_MAIN
            ? CRS_DBG_MAIN
            : CRS_DBG_VM;

        if (func->flags & FUNC_DEBUG) {
            return func->debug.source->contents;
        }
    }

    *what = CRS_DBG_C;
    return "?";
}

int crsD_getParams(crs_Frame* frame) {
    return frame->flags & CALL_VM
        ? frame->i.v.f->args
        : 0;
}

int crsD_getLine(crs_Frame* frame) {
    if (!(frame->flags & CALL_VM)) {
        return 0;
    }

    crs_Function* func  = frame->i.v.f;
    Debug_Info*   debug = &func->debug;
    Debug_Line*   info  = debug->lines;
    unsigned      low   = 0;
    unsigned      high  = debug->nL - 1;
    unsigned      pc    = (unsigned)(frame->i.v.pc - func->code);

    while (low <= high) {
        unsigned mid = low + (high - low) / 2;
        info         = &debug->lines[mid];

        if (low == high) {
            break; /* found it */
        }

        if (info->pc > pc) {
            high = mid - 1;
        } else if ((info + 1)->pc <= pc) {
            /* info->pc <= pc >= (info + 1)->pc */
            low = mid + 1;
        } else {
            /* info->pc <= pc < (info + 1)->pc */
            break;
        }
    }

    return info->line;
}

void crsD_getFunc(crs_Frame* frame, crs_Object* object) {
    if (frame->flags & CALL_VM) {
        obj_setgc(object, frame->i.v.f);
    } else {
        obj_setc(object, frame->i.c.f);
    }
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
