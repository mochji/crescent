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
                                           unsigned nN) {
    crs_Function* func = mem_new(thread, crs_Function);

    if (func == NULL) {
        crsM_error(thread);
    }

    func->flags  = 0;
    func->args   = 0;
    func->top    = 0;
    func->nI     = nI;
    func->nC     = nC;
    func->nN     = nN;
    func->cC     = 0;
    func->cN     = 0;
    func->code   = NULL;
    func->consts = NULL;
    func->nested = NULL;

    func->code   = tryBlock(thread, func, nI, sizeof(crs_instr));
    func->consts = tryBlock(thread, func, nC, sizeof(crs_Object));
    func->nested = tryBlock(thread, func, nN, sizeof(crs_Function*));

    return crsG_add(thread, func, CRS_TYPE_FUNCTION);
}

void crsK_free(crs_Thread* thread, crs_Function* func) {
    mem_vfree(thread, func->code, func->nI);
    mem_vfree(thread, func->consts, func->nC);
    mem_vfree(thread, func->nested, func->nN);
    mem_free(thread, func);
}

/*
 * Bytecode dump format
 *
 * WARNING: Bytecode dumps are not portable across different versions and
 *          configurations of Crescent.
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
 * }
 *
 * constant {
 *   byte         | type (integer)
 *   crs_Integer  | value
 * }
 * ... or ...
 * constant {
 *   byte         | type (float)
 *   crs_Float    | value
 * }
 * ... or ...
 * constant {
 *   byte         | type (string)
 *   crs_Integer  | length
 *   char[length] | value
 * }
 */

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

static void dump_const(crs_Dump* dump, crs_Object* object) {
    dump_value(dump, object->type, crs_byte);

    switch (object->type) {
        case CRS_TYPE_INTEGER:
            dump_value(dump, obj_geti(object), crs_Integer);
            break;
        case CRS_TYPE_FLOAT:
            dump_value(dump, obj_getf(object), crs_Float);
            break;
        case CRS_TYPE_STRING: {
            crs_String* string = obj_gets(object);
            dump_value(dump, string->length, crs_Integer);
            crsW_write(dump, string->contents, (size_t)string->length);

            break;
        }
        default:
            assert(0);
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
}

typedef struct {
    crs_Dump*     dump;
    crs_Function* func;
} DumpInfo;

static void* dump_try(crs_Thread* thread, void* data) {
    UNUSED(thread);
    DumpInfo* info = data;
    crs_Dump* dump = info->dump;

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

    return crsC_try(dump->thread, &dump_try, &info, NULL);
}

/*
 * ===========================
 *  loading
 * ===========================
 */

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

static unsigned load_unsigned(crs_Stream* stream) {
    unsigned value;
    load_block(stream, (char*)&value, sizeof(unsigned));

    return value;
}

static crs_Integer load_int(crs_Stream* stream) {
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

static crs_Integer load_length(crs_Stream* stream) {
    crs_Integer length = load_int(stream);

    if (length < 0) {
        load_error(stream, "corrupted dump");
    } else if ((crs_Unsigned)length > SIZE_MAX) {
        load_error(stream, "string too long");
    }

    return length;
}

static void load_checkHeader(crs_Stream* stream) {
    load_checkString(stream, CRS_SIGNATURE, 4, "not a binary dump");
    load_checkString(stream, CRS_DUMPCHECK, 8, "corrupted dump");
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
            crs_Integer value = load_int(stream);
            obj_seti(object, value);
            break;
        }
        case CRS_TYPE_FLOAT: {
            crs_Float value = load_float(stream);
            obj_setf(object, value);
            break;
        }
        case CRS_TYPE_STRING: {
            size_t      length = (size_t)load_length(stream);
            crs_String* string = crsS_newo(stream->thread, length);
            obj_setgc(object, string);
            load_block(stream, string->contents, length);
            break;
        }
        default:
            load_error(stream, "corrupted dump");
    }
}

static crs_Function* load_func(crs_Stream* stream, crs_Function* parent) {
    crs_Thread*   thread = stream->thread;
    crs_Function* func;

    unsigned nI = load_size(stream, sizeof(crs_instr), "instructions");
    unsigned nC = load_size(stream, sizeof(crs_Object), "constants");
    unsigned nN = load_size(stream, sizeof(crs_Function*), "nested functions");
    func        = crsK_new(thread, nI, nC, nN);

    if (parent != NULL) {
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

    if (parent == NULL) {
        crsC_unanchor(thread);
    }

    return func;
}

crs_Function* crsK_load(crs_Stream* stream) {
    load_checkHeader(stream);
    return load_func(stream, NULL);
}
