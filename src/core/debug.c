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

#include "types/string.h"
#include "core/object.h"
#include "core/state.h"
#include "core/format.h"
#include "core/buffer.h"
#include "core/methods.h"
#include "vm/opcodes.h"

#include "core/debug.h"

static char* fromVar(crs_Function* caller, crs_byte reg, unsigned pc,
                                           int* from) {
    Debug_Info* debug = &caller->debug;
    Debug_Var*  var   = debug->vars;

    for (unsigned i = 0; i < debug->nV; i++) {
        unsigned start = var->start;
        unsigned end   = var->end;

        if (pc >= start && pc < end && var->reg == reg) {
            *from = var->type;
            return var->name->contents;
        }

        var++;
    }

    *from = CRS_FROM_UNKNOWN;
    return "?";
}

static char* getName(crs_Function* caller, crs_instr* i, int* from) {
    unsigned pc = (unsigned)(i - caller->code);

    switch (instr_opcode(*i)) {
        case OP_UNM:
            *from = CRS_FROM_MM;
            return crsM_names[MT_UNM];
        case OP_ADD:
            *from = CRS_FROM_MM;
            return crsM_names[MT_ADD];
        case OP_SUB:
            *from = CRS_FROM_MM;
            return crsM_names[MT_SUB];
        case OP_MUL:
            *from = CRS_FROM_MM;
            return crsM_names[MT_MUL];
        case OP_DIV:
            *from = CRS_FROM_MM;
            return crsM_names[MT_DIV];
        case OP_POW:
            *from = CRS_FROM_MM;
            return crsM_names[MT_POW];
        case OP_MOD:
            *from = CRS_FROM_MM;
            return crsM_names[MT_MOD];
        case OP_BNOT:
            *from = CRS_FROM_MM;
            return crsM_names[MT_BNOT];
        case OP_BAND:
            *from = CRS_FROM_MM;
            return crsM_names[MT_BAND];
        case OP_BOR:
            *from = CRS_FROM_MM;
            return crsM_names[MT_BOR];
        case OP_BXOR:
            *from = CRS_FROM_MM;
            return crsM_names[MT_BXOR];
        case OP_SHL:
            *from = CRS_FROM_MM;
            return crsM_names[MT_SHL];
        case OP_SHR:
            *from = CRS_FROM_MM;
            return crsM_names[MT_SHR];
        case OP_EQ:
            *from = CRS_FROM_MM;
            return crsM_names[MT_EQ];
        case OP_LE:
            *from = CRS_FROM_MM;
            return crsM_names[MT_LE];
        case OP_LT:
            *from = CRS_FROM_MM;
            return crsM_names[MT_LT];
        case OP_GE:
            *from = CRS_FROM_MM;
            return crsM_names[MT_GE];
        case OP_GT:
            *from = CRS_FROM_MM;
            return crsM_names[MT_GT];
        case OP_LENGTH:
            *from = CRS_FROM_MM;
            return crsM_names[MT_LEN];
        case OP_CONCAT:
            *from = CRS_FROM_MM;
            return crsM_names[MT_CONCAT];
        case OP_GET:
            *from = CRS_FROM_MM;
            return crsM_names[MT_GET];
        case OP_SET:
            *from = CRS_FROM_MM;
            return crsM_names[MT_SET];
        case OP_CALL:
            if (!func_hasdebug(caller)) {
                break;
            }

            return fromVar(caller, instr_A(*i), pc, from);
    }

    *from = CRS_FROM_UNKNOWN;
    return "?";
}

static int getLine(Debug_Info* debug, unsigned pc) {
    Debug_Line* line = debug->lines;
    unsigned    low  = 0;
    unsigned    high = debug->nL - 1;

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

char* crsD_source(crs_Frame* frame, int* what) {
    if (!call_isvm(frame)) {
        *what = CRS_WHAT_C;
        return "[C]";
    }

    crs_Function* func = frame->i.v.f;
    *what              = func_ismain(func) ? CRS_WHAT_MAIN : CRS_WHAT_VM;

    return func_hasdebug(func)
        ? func->debug.source->contents
        : "?";
}

char* crsD_name(crs_Frame* frame, int* from) {
    crs_Frame* caller = frame->previous;

    if (!call_isvm(caller)) {
        *from = CRS_FROM_UNKNOWN;
        return "?";
    }

    return getName(caller->i.v.f, caller->i.v.pc, from);
}

int crsD_params(crs_Frame* frame) {
    if (call_isvm(frame)) {
        return frame->i.v.f->args;
    } else {
        return 0;
    }
}

int crsD_line(crs_Frame* frame) {
    if (call_hasdebug(frame)) {
        crs_Function* func = frame->i.v.f;
        unsigned      pc   = (unsigned)(frame->i.v.pc - func->code);
        return getLine(&func->debug, pc);
    } else {
        return 0;
    }
}

void crsD_func(crs_Frame* frame, crs_Object* obj) {
    if (call_isvm(frame)) {
        obj_setgc(obj, frame->i.v.f);
    } else {
        obj_setc(obj, frame->i.c.f);
    }
}

crs_Object* crsD_getLocal(crs_Frame* frame, crs_String* name) {
    if (!call_hasdebug(frame)) {
        return NULL;
    }

    crs_Function* func  = frame->i.v.f;
    Debug_Info*   debug = &func->debug;
    Debug_Var*    var   = debug->vars;
    unsigned      pc    = (unsigned)(frame->i.v.pc - func->code);

    for (unsigned i = 0; i < debug->nV; i++) {
        unsigned start   = var->start;
        unsigned end     = var->end;
        int      isLocal = pc >= start && pc < end && var->type == DVAR_LOCAL;

        if (isLocal && crsS_equal(var->name, name)) {
            return frame->base + var->reg;
        }

        var++;
    }

    return NULL;
}

void crsD_warn(crs_Thread* thread, char* msg) {
    crs_State* state = thread->state;

    if (state->debug.warnF != NULL) {
        state->debug.warnF(msg, state->debug.warnD);
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
