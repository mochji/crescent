/*
 * https://github.com/mochji/crescent
 * types/function.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>

#include "crescent/conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/table.h"
#include "core/object.h"
#include "core/memory.h"
#include "core/buffer.h"
#include "core/format.h"
#include "core/call.h"
#include "core/debug.h"
#include "core/gc.h"

#include "types/function.h"

static void* tryBlock(crs_Thread* thread, crs_Function* func,
                                          unsigned count, size_t size) {
    void* block;

    if (!count) {
        return NULL;
    } else if ((block = mem_alloc(thread, count * size)) == NULL) {
        crsK_free(thread, func);
        crsM_error(thread);
    }

    return block;
}

crs_Function* crsK_new(crs_Thread* thread, unsigned nI, unsigned nC,
                                           unsigned nN, int debug) {
    crs_Function* func = mem_new(thread, crs_Function);

    if (func == NULL) {
        crsM_error(thread);
    }

    func->flags        = 0;
    func->args         = 0;
    func->top          = 0;
    func->nI           = nI;
    func->nC           = nC;
    func->nN           = nN;
    func->cC           = 0;
    func->cN           = 0;
    func->code         = NULL;
    func->consts       = NULL;
    func->nested       = NULL;
    func->debug.source = thread->state->memoryError; /* placeholder */
    func->debug.lines  = NULL;
    func->debug.vars   = NULL;
    func->debug.nL     = 0;
    func->debug.nV     = 0;
    func->debug.cV     = 0;

    func->code   = tryBlock(thread, func, nI, sizeof(crs_instr));
    func->consts = tryBlock(thread, func, nC, sizeof(crs_Object));
    func->nested = tryBlock(thread, func, nN, sizeof(crs_Function*));

    if (debug) {
        func->flags      |= FUNC_DEBUG;
        func->debug.nL    = 16;
        func->debug.nV    = 16;
        func->debug.lines = tryBlock(thread, func, 16, sizeof(Debug_Line));
        func->debug.vars  = tryBlock(thread, func, 16, sizeof(Debug_Var));
    }

    return crsG_add(thread, func, CRS_TYPE_FUNCTION);
}

void crsK_free(crs_Thread* thread, crs_Function* func) {
    mem_vfree(thread, func->code, func->nI);
    mem_vfree(thread, func->consts, func->nC);
    mem_vfree(thread, func->nested, func->nN);
    mem_vfree(thread, func->debug.lines, func->debug.nL);
    mem_vfree(thread, func->debug.vars, func->debug.nV);
    mem_free(thread, func);
}

/*
 * Bytecode dump format
 *
 * WARNING: Bytecode dumps are not portable across different versions and
 *          configurations of Crescent.
 *
 * dump {
 *   global header | global header
 *   function      | main function
 * }
 *
 * global header {
 *   char[4] | signature
 *   char[8] | decoding check
 *   byte    | crescent version
 *   byte    | endianness (0 = big endian; 1 = little endian)
 *   byte    | sizeof(unsigned)
 *   byte    | sizeof(crs_instr)
 *   byte    | sizeof(crs_Integer)
 *   byte    | sizeof(crs_Float)
 * }
 *
 * function {
 *   unsigned | # of instructions
 *   unsigned | # of constants
 *   unsigned | # of nested functions
 *   byte     | flags
 *   byte     | args
 *   byte     | top
 *
 *   crs_instr[nI] | code
 *   constant[nC]  | constants
 *   function[nN]  | nested functions
 *   if flags & FLAG_DEBUG {
 *     debug       | debug info
 *   }
 * }
 *
 * string {
 *   crs_Integer    | length (if < 0, index into string table)
 *   if length >= 0 {
 *     char[length] | contents
 *   }
 * }
 *
 * constant {
 *   byte             | type
 *   switch type {
 *     integer:
 *       crs_Integer  | value
 *     float:
 *       crs_Float    | value
 *     string:
 *       string       | value
 *   }
 * }
 *
 * debug {
 *   string      | source name
 *   unsigned    | # of lines (always at least one)
 *   unsigned    | # of vars (zero or more)
 *   line[lines] | line information
 *   var[vars]   | variable information
 * }
 *
 * line {
 *   byte       | info
 *   - bit  0   | is relative pc
 *   - bits 1-7 | line # offset (must increment; if zero, then absolute)
 *   if info.0 {
 *     byte     | pc offset - 1 (must increment)
 *   } else {
 *     unsigned | absolute pc
 *   }
 *   if !info.1-7 {
 *     int      | absolute line #
 *   }
 * }
 *
 * var {
 *   byte       | type
 *   - bits 0-1 | variable type
 *   - bit  2   | start pc relative to previous start
 *   - bits 3-7 | end pc relative to start (if zero, then absolute)
 *   byte       | register
 *   if type.2 {
 *     byte     | start pc offset (can be same)
 *   } else {
 *     unsigned | absolute start pc
 *   }
 *   if !type.3-7 {
 *     unsigned | absolute end pc
 *   }
 *   string     | name
 * }
 */

#define LINE_RELPC      0x01
#define LINE_RELLINE    0xFE
#define LINE_MAXRELPC   0x100
#define LINE_MAXRELLINE 0x7F

#define VAR_TYPE        0x03
#define VAR_RELSTART    0x04
#define VAR_RELEND      0xF8
#define VAR_MAXRELSTART 0x100
#define VAR_MAXRELEND   0x1F

static crs_byte endianness(void) {
    int dummy = 1;
    return *((crs_byte*)&dummy);
}

/*
 * ===========================
 *  dumping
 * ===========================
 */

#define dump_value(d, v, t) {t x = (t)(v); crsW_write(d, (char*)&x, sizeof(t));}

static void dump_header(crs_Dump* dump) {
    crsW_write(dump, CRS_SIGNATURE, 4);
    crsW_write(dump, CRS_DUMPCHECK, 8);
    dump_value(dump, CRS_VERSION, crs_byte);
    dump_value(dump, endianness(), crs_byte);
    dump_value(dump, sizeof(unsigned), crs_byte);
    dump_value(dump, sizeof(crs_instr), crs_byte);
    dump_value(dump, sizeof(crs_Integer), crs_byte);
    dump_value(dump, sizeof(crs_Float), crs_byte);
}

/*
 * strings are dumped in the order that they are encountered, and so they are
 * loaded in the same order.
 */
static void dump_string(crs_Dump* dump, crs_String* string) {
    crs_Thread* thread  = dump->thread;
    crs_Table*  strings = obj_gett(thread->stack.top - 1);
    crs_Object  key;

    obj_setgc(&key, string);
    crs_Object* result = crsT_get(thread, strings, &key);

    if (result->type != CRS_TYPE_NIL) {
        /* string already exists; reuse it */
        dump_value(dump, -obj_geti(result) - 1, crs_Integer);
    } else {
        /* new string */
        dump_value(dump, string->length, crs_Integer);
        crsW_write(dump, string->contents, (size_t)string->length);
        crsT_set(thread, strings, &key, thread->stack.top - 2);
        obj_seti(thread->stack.top - 2, obj_geti(thread->stack.top - 2) + 1);
    }
}

static void dump_const(crs_Dump* dump, crs_Object* object) {
    dump_value(dump, object->type, crs_byte);

    switch (object->type) {
        case CRS_TYPE_INTEGER:
            dump_value(dump, obj_geti(object), crs_Integer);
            break;
        case CRS_TYPE_FLOAT:
            dump_value(dump, obj_getf(object), crs_Float);
            break;
        case CRS_TYPE_STRING:
            dump_string(dump, obj_gets(object));
            break;
        default:
            assert(0);
    }
}

static Debug_Line* dump_line(crs_Dump* dump, Debug_Line* prev,
                                             Debug_Line* line) {
    crs_byte info = 0;
    unsigned pc   = line->pc;
    int      num  = line->line;

    if (prev != NULL) {
        unsigned pcDiff  = pc - prev->pc;
        int      numDiff = num - prev->line;

        if (pcDiff <= LINE_MAXRELPC) {
            pc    = pcDiff - 1;
            info |= LINE_RELPC;
        }

        if (numDiff <= LINE_MAXRELLINE) {
            info |= (crs_byte)numDiff << 1;
        }
    }

    dump_value(dump, info, crs_byte);

    if (info & LINE_RELPC) {
        dump_value(dump, pc, crs_byte);
    } else {
        dump_value(dump, pc, unsigned);
    }

    if (!(info & LINE_RELLINE)) {
        dump_value(dump, num, int);
    }

    return line;
}

static Debug_Var* dump_var(crs_Dump* dump, Debug_Var* prev, Debug_Var* var) {
    crs_byte type  = var->type;
    unsigned start = var->start;
    unsigned end   = var->end;

    if (prev != NULL) {
        unsigned startDiff = start - prev->start;
        unsigned endDiff   = end - start;

        if (startDiff <= VAR_MAXRELSTART) {
            type |= VAR_RELSTART;
            start = startDiff;
        }

        if (endDiff <= VAR_MAXRELEND) {
            type |= (crs_byte)endDiff << 3;
        }
    }

    dump_value(dump, type, crs_byte);
    dump_value(dump, var->reg, crs_byte);

    if (type & VAR_RELSTART) {
        dump_value(dump, start, crs_byte);
    } else {
        dump_value(dump, start, unsigned);
    }

    if (!(type & VAR_RELEND)) {
        dump_value(dump, end, unsigned);
    }

    dump_string(dump, var->name);
    return var;
}

static void dump_debug(crs_Dump* dump, crs_Function* func) {
    if (!(func->flags & FUNC_DEBUG)) {
        return;
    }

    Debug_Info* debug    = &func->debug;
    Debug_Line* prevLine = NULL;
    Debug_Var*  prevVar  = NULL;

    dump_string(dump, debug->source);
    dump_value(dump, debug->nL, unsigned);
    dump_value(dump, debug->nV, unsigned);

    for (unsigned i = 0; i < debug->nL; i++) {
        prevLine = dump_line(dump, prevLine, &debug->lines[i]);
    }

    for (unsigned i = 0; i < debug->nV; i++) {
        prevVar = dump_var(dump, prevVar, &debug->vars[i]);
    }
}

static void dump_func(crs_Dump* dump, crs_Function* func) {
    dump_value(dump, func->nI, unsigned);
    dump_value(dump, func->nC, unsigned);
    dump_value(dump, func->nN, unsigned);
    dump_value(dump, func->flags, crs_byte);
    dump_value(dump, func->args, crs_byte);
    dump_value(dump, func->top, crs_byte);
    crsW_write(dump, (char*)func->code, func->nI * sizeof(crs_instr));

    for (unsigned i = 0; i < func->nC; i++) {
        dump_const(dump, &func->consts[i]);
    }

    for (unsigned i = 0; i < func->nN; i++) {
        dump_func(dump, func->nested[i]);
    }

    dump_debug(dump, func);
}

typedef struct {
    crs_Dump*     dump;
    crs_Function* func;
} DumpInfo;

static void* dump_try(crs_Thread* thread, void* data) {
    UNUSED(thread);
    DumpInfo* info = data;
    crs_Dump* dump = info->dump;

    obj_seti(thread->stack.top, 0); /* # of strings */
    thread->stack.top++;
    crsC_anchor(thread, obj_toheader(crsT_new(thread))); /* string table */
    dump_header(dump);
    dump_func(dump, info->func);
    crsW_flush(dump);

    return NULL;
}

int crsK_dump(crs_Dump* dump, crs_Function* func) {
    DumpInfo info = {
        .dump = dump,
        .func = func
    };

    crs_Thread* thread = dump->thread;
    ptrdiff_t   top    = call_savetop(thread);
    int         status = crsC_try(thread, &dump_try, &info, NULL);
    call_restoretop(thread, top);

    return status;
}

/*
 * ===========================
 *  loading
 * ===========================
 */

/* general message for unexpected data */
#define LOAD_CORRUPTED "corrupted dump"

static noret load_error(crs_Stream* stream, char* format, ...) {
    crs_Thread* thread = stream->thread;
    crs_String* error;
    va_list     args;

    va_start(args, format);
    error = crsF_vformat(thread, format, args);
    va_end(args);

    obj_setgc(&thread->error, error);
    crsC_throw(thread, CRS_CODEERR);
}

static void load_block(crs_Stream* stream, char* buffer, size_t count) {
    if (count > crsR_read(stream, buffer, count)) {
        load_error(stream, "truncated dump");
    }
}

static crs_byte load_byte(crs_Stream* stream) {
    crs_byte value;
    load_block(stream, (char*)&value, sizeof(crs_byte));

    return value;
}

static int load_int(crs_Stream* stream) {
    int value;
    load_block(stream, (char*)&value, sizeof(int));

    return value;
}

static unsigned load_unsigned(crs_Stream* stream) {
    unsigned value;
    load_block(stream, (char*)&value, sizeof(unsigned));

    return value;
}

static crs_Integer load_integer(crs_Stream* stream) {
    crs_Integer value;
    load_block(stream, (char*)&value, sizeof(crs_Integer));

    return value;
}

static crs_Float load_float(crs_Stream* stream) {
    crs_Float value;
    load_block(stream, (char*)&value, sizeof(crs_Float));

    return value;
}

static void load_checkByte(crs_Stream* stream, crs_byte expected, char* what) {
    if (load_byte(stream) != expected) {
        load_error(stream, "%s mismatch", what);
    }
}

static void load_checkString(crs_Stream* stream, char* str, int length,
                                                 char* error) {
    char c;

    while (length--) {
        load_block(stream, &c, sizeof(char));

        if (c != *str++) {
            load_error(stream, error);
        }
    }
}

static unsigned load_size(crs_Stream* stream, size_t size, char* what) {
    unsigned count = load_unsigned(stream);

    if (count > SIZE_MAX / size) {
        load_error(stream, "too many %s", what);
    }

    return count;
}

static crs_String* load_string(crs_Stream* stream) {
    crs_Thread* thread  = stream->thread;
    crs_Table*  strings = obj_gett(thread->stack.top - 2); /* -1 is main func */
    crs_Integer length  = load_integer(stream);
    crs_String* string;
    crs_Object  key;

    if (length >= 0) {
        /* new string */
        if ((crs_Unsigned)length > SIZE_MAX) {
            load_error(stream, "string too long");
        }

        string            = crsS_newo(thread, (size_t)length);
        crs_Object* value = crsC_anchor(thread, obj_toheader(string));
        load_block(stream, string->contents, (size_t)length);

        obj_seti(&key, strings->length);
        crsT_set(thread, strings, &key, value);

        crsC_unanchor(thread);
    } else {
        /* reuse string */
        obj_seti(&key, -length - 1);
        crs_Object* result = crsT_get(thread, strings, &key);

        if (result->type == CRS_TYPE_NIL) {
            load_error(stream, LOAD_CORRUPTED);
        } else {
            string = obj_gets(result);
        }
    }

    return string;
}

static void load_checkHeader(crs_Stream* stream) {
    load_checkString(stream, CRS_SIGNATURE, 4, "not a binary dump");
    load_checkString(stream, CRS_DUMPCHECK, 8, LOAD_CORRUPTED);
    load_checkByte(stream, CRS_VERSION, "version");
    load_checkByte(stream, endianness(), "endianness");
    load_checkByte(stream, sizeof(unsigned), "unsigned size");
    load_checkByte(stream, sizeof(crs_instr), "instruction size");
    load_checkByte(stream, sizeof(crs_Integer), "integer size");
    load_checkByte(stream, sizeof(crs_Float), "float size");
}

static void load_const(crs_Stream* stream, crs_Object* object) {
    switch (load_byte(stream)) {
        case CRS_TYPE_INTEGER: {
            crs_Integer value = load_integer(stream);
            obj_seti(object, value);
            break;
        }
        case CRS_TYPE_FLOAT: {
            crs_Float value = load_float(stream);
            obj_setf(object, value);
            break;
        }
        case CRS_TYPE_STRING: {
            crs_String* string = load_string(stream);
            obj_setgc(object, string);
            break;
        }
        default:
            load_error(stream, LOAD_CORRUPTED);
    }
}

static Debug_Line* load_line(crs_Stream* stream, Debug_Line* prev,
                                                 Debug_Line* line) {
    crs_byte info = load_byte(stream);

    if (prev == NULL && (info & (LINE_RELPC | LINE_RELLINE))) {
        /* first must be absolute */
        load_error(stream, LOAD_CORRUPTED);
    }

    if (info & LINE_RELPC) {
        line->pc = (prev->pc + load_byte(stream)) + 1;
    } else {
        line->pc = load_unsigned(stream);
    }

    if (info & LINE_RELLINE) {
        line->line = prev->line + ((info & LINE_RELLINE) >> 1);
    } else {
        line->line = load_int(stream);
    }

    return line;
}

static Debug_Var* load_var(crs_Stream* stream, Debug_Var* prev,
                                               Debug_Var* var) {
    crs_byte type = load_byte(stream);
    var->reg      = load_byte(stream);
    var->type     = type;

    if (prev == NULL && (type & (VAR_RELSTART | VAR_RELEND))) {
        /* first must be absolute */
        load_error(stream, LOAD_CORRUPTED);
    }

    if (type & VAR_RELSTART) {
        var->start = prev->start + load_byte(stream);
    } else {
        var->start = load_unsigned(stream);
    }

    if (type & VAR_RELEND) {
        var->end = var->start + ((type & VAR_RELEND) >> 3);
    } else {
        var->end = load_unsigned(stream);
    }

    var->name = load_string(stream);
    return var;
}

static void load_debug(crs_Stream* stream, crs_Function* func) {
    if (!(func->flags & FUNC_DEBUG)) {
        return;
    }

    Debug_Info* debug = &func->debug;
    debug->source     = load_string(stream);
    debug->nL         = load_size(stream, sizeof(Debug_Line), "lines");
    debug->nV         = load_size(stream, sizeof(Debug_Var), "variables");
    debug->lines      = mem_vnew(stream->thread, debug->nL, Debug_Line);
    debug->vars       = debug->nV
        ? mem_vnew(stream->thread, debug->nV, Debug_Var)
        : NULL;

    if (debug->lines == NULL || (debug->nV && debug->vars == NULL)) {
        crsM_error(stream->thread);
    }

    Debug_Line* prevLine = NULL;
    Debug_Var*  prevVar  = NULL;

    for (unsigned i = 0; i < debug->nL; i++) {
        prevLine = load_line(stream, prevLine, &debug->lines[i]);
    }

    for (unsigned i = 0; i < debug->nV; i++) {
        prevVar = load_var(stream, prevVar, &debug->vars[i]);
        debug->cV++;
    }
}

static crs_Function* load_func(crs_Stream* stream, crs_Function* parent) {
    crs_Thread*   thread = stream->thread;
    crs_Function* func;

    unsigned nI = load_size(stream, sizeof(crs_instr), "instructions");
    unsigned nC = load_size(stream, sizeof(crs_Object), "constants");
    unsigned nN = load_size(stream, sizeof(crs_Function*), "nested functions");
    func        = crsK_new(thread, nI, nC, nN, 0);

    if (parent != NULL) {
        func->flags                 |= FUNC_MAIN;
        parent->nested[parent->cN++] = func;
    } else {
        crsC_anchor(thread, obj_toheader(func));
    }

    func->flags = load_byte(stream);
    func->args  = load_byte(stream);
    func->top   = load_byte(stream);

    crsR_read(stream, (char*)func->code, nI * sizeof(crs_instr));

    while (func->cC < nC) {
        crs_Object* object = &func->consts[func->cC++];
        obj_setn(object);
        load_const(stream, object);
    }

    while (func->cN < nN) {
        load_func(stream, func);
    }

    load_debug(stream, func);

    if (parent == NULL) {
        crsC_unanchor(thread);
    }

    return func;
}

crs_Function* crsK_load(crs_Stream* stream) {
    crs_Thread*   thread = stream->thread;
    crs_Function* main;

    crsC_anchor(thread, obj_toheader(crsT_new(thread))); /* string table */
    load_checkHeader(stream);
    main = load_func(stream, NULL);

    return main;
}
