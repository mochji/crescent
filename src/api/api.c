/*
 * https://github.com/mochji/crescent
 * api/api.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdio.h>
#include <stddef.h>
#include <limits.h>

#include "crescent/conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/table.h"
#include "types/function.h"
#include "core/object.h"
#include "core/buffer.h"
#include "core/format.h"
#include "core/state.h"
#include "core/memory.h"
#include "core/call.h"
#include "core/debug.h"
#include "core/gc.h"
#include "compiler/parser.h"
#include "vm/vm.h"

#include "crescent/api.h"

static int panic(crs_Thread* thread) {
    crs_Object* error = &thread->error;
    char*       message;

    if (error->type == CRS_TYPE_STRING) {
        message = obj_gets(error)->contents;
    } else {
        message = "error is not a string";
    }

    fprintf(stderr,
        "PANIC: error within unprotected call to Crescent API (%s)\n", message);

    return 0;
}

static crs_Object* getIndex(crs_Thread* thread, int index) {
    crs_Object* nil = &crsO_nilValue;
    crs_Object* object;

    if (index == 0) {
        return nil;
    } else if (index == CRS_GLOBALS) {
        return &thread->state->globals;
    }

    if (index < 0) {
        object = thread->stack.top + index;

        return object >= thread->stack.frame->base
            ? object
            : nil;
    }

    object = thread->stack.frame->base + index - 1;

    return object < thread->stack.top
        ? object
        : nil;
}

static crs_Object* adjustTop(crs_Thread* thread, int amount) {
    crs_Frame* frame = thread->stack.frame;
    int        items = (int)(thread->stack.top - frame->base);

    if (-amount > items) {
        amount = -items;
    } else if (items + amount > frame->top) {
        crsC_error(thread, "stack overflow");
    }

    thread->stack.top += amount;
    return thread->stack.top - 1;
}

export int crs_version(void) {
    return CRS_VERSION;
}

export int crs_release(void) {
    return CRS_RELEASE;
}

/*
 * ===========================
 *  state manipulation
 * ===========================
 */

export crs_Thread* crs_open(void) {
    crs_Thread* thread = crsE_open();

    if (thread == NULL) {
        return NULL;
    }

    thread->state->panic = &panic;

    return thread;
}

export void crs_close(crs_Thread* thread) {
    crsE_close(thread->state);
}

export void crs_setPanic(crs_Thread* thread, crs_CFunction* function) {
    thread->state->panic = function;
}

export void crs_error(crs_Thread* thread, int index) {
    crs_Object* object = getIndex(thread, index);

    obj_seto(&thread->error, object);
    crsC_throw(thread, CRS_ERROR);
}

/*
 * ===========================
 *  gc management
 * ===========================
 */

export int crs_gc(crs_Thread* thread, int option) {
    switch (option) {
        case CRS_GC_STEP:
            return crsG_step(thread);
        case CRS_GC_FULL:
            crsG_full(thread, 0); return 1;
        case CRS_GC_USAGE:
            return (int)(thread->state->gc.usage / 1024);
    }

    return -1;
}

export int crs_getGC(crs_Thread* thread, int option) {
    crs_State* state = thread->state;

    switch (option) {
        case CRS_GC_STOP:
            return gc_getstatus(state, STOP);
        case CRS_GC_STOPEM:
            return gc_getstatus(state, STOPEM);
        case CRS_GC_PAUSE:
            return gc_getparam(state, PAUSE);
        case CRS_GC_STEP:
            return gc_getparam(state, STEP);
        case CRS_GC_MULTIPLIER:
            return gc_getparam(state, MULTIPLIER);
    }

    return -1;
}

export void crs_setGC(crs_Thread* thread, int option, unsigned short value) {
    crs_State* state = thread->state;

    switch (option) {
        case CRS_GC_STOP:
            value = value != 0;
            gc_setstatus(state, STOP, value);

            break;
        case CRS_GC_STOPEM:
            value = value != 0;
            gc_setstatus(state, STOPEM, value);

            break;
        case CRS_GC_PAUSE:
            gc_setparam(state, PAUSE, value);

            break;
        case CRS_GC_STEP:
            gc_setparam(state, STEP, value);

            break;
        case CRS_GC_MULTIPLIER:
            gc_setparam(state, MULTIPLIER, value);

            break;
    }
}

/*
 * ===========================
 *  stack manipulation
 * ===========================
 */

export int crs_getTop(crs_Thread* thread) {
    return (int)(thread->stack.top - thread->stack.frame->base);
}

export int crs_checkTop(crs_Thread* thread, int top) {
    int result = !crsC_checkTop(thread, top, 0);
    crsG_check(thread);

    return result;
}

export void crs_setTop(crs_Thread* thread, int top) {
    if (top < 0) {
        top = 0;
    } else if (top > CRS_MAX_TOP) {
        top = CRS_MAX_TOP;
    }

    crs_Frame* frame = thread->stack.frame;

    if (top > frame->top) {
        crsC_error(thread, "stack overflow");
    }

    crs_Object* object = thread->stack.top;
    crs_Object* to     = frame->base + top;

    if (object < to) {
        while (object < to) {
            obj_setn(object);
            object++;
        }
    }

    thread->stack.top = to;
}

/* pop a max of 'amount' objects off of the stack */
export void crs_pop(crs_Thread* thread, int amount) {
    if (amount <= 0) {
        return;
    }

    int items = (int)(thread->stack.top - thread->stack.frame->base);

    if (amount > items) {
        amount = items;
    }

    thread->stack.top -= amount;
}

/* remove an element from the stack, shifting above elements down */
export void crs_remove(crs_Thread* thread, int index) {
    if (index == 0) {
        return;
    }

    crs_Frame* frame = thread->stack.frame;
    int        items = (int)(thread->stack.top - frame->base);

    if (index > items) {
        return;
    } else if (index < 0) {
        if (-index > items) {
            return;
        }

        index += items + 1;
    }

    crs_Object* object = frame->base + index - 1;

    while (object < thread->stack.top) {
        obj_seto(object, object + 1);
        object++;
    }

    thread->stack.top -= 1;
}

/*
 * ===========================
 *  basic object functions
 * ===========================
 */

export int crs_type(crs_Thread* thread, int index) {
    return getIndex(thread, index)->type;
}

export const char* crs_name(crs_Thread* thread, int index) {
    return crsO_name(getIndex(thread, index));
}

/*
 * ===========================
 *  operations
 * ===========================
 */

export crs_Integer crs_length(crs_Thread* thread, int index) {
    return crsV_length(thread, getIndex(thread, index));
}

export int crs_compare(crs_Thread* thread, int leftIndex, int rightIndex,
                                           int op) {
    crs_Object* left  = getIndex(thread, leftIndex);
    crs_Object* right = getIndex(thread, rightIndex);

    if (op == CRS_OP_EQ) {
        return crsV_equal(left, right);
    }

    return crsV_compare(thread, left, right, op);
}

export void crs_arith(crs_Thread* thread, int leftIndex, int rightIndex,
                                          int op) {
    crs_Object* left  = getIndex(thread, leftIndex);
    crs_Object* right = getIndex(thread, rightIndex);

    crsV_arith(thread, adjustTop(thread, 1), left, right, op);
}

export void crs_get(crs_Thread* thread, int index, int keyIndex) {
    crs_Object* key    = getIndex(thread, keyIndex);
    crs_Object* value  = crsV_get(thread, getIndex(thread, index), key);
    crs_Object* object = adjustTop(thread, 1);

    obj_seto(object, value);
    crsG_check(thread);
}

export void crs_set(crs_Thread* thread, int index, int keyIndex,
                                        int valueIndex) {
    crs_Object* key    = getIndex(thread, keyIndex);
    crs_Object* value  = getIndex(thread, valueIndex);

    crsV_set(thread, getIndex(thread, index), key, value);
    crsG_check(thread);
}

export void crs_copy(crs_Thread* thread, int index) {
    crs_Object* from = getIndex(thread, index);
    crs_Object* to   = adjustTop(thread, 1);

    obj_seto(to, from);
}

/*
 * ===========================
 *  is* functions
 * ===========================
 */

export int crs_isNumber(crs_Thread* thread, int index) {
    return obj_isnumber(getIndex(thread, index));
}

/*
 * ===========================
 *  to* functions
 * ===========================
 */

export int crs_toBooleanX(crs_Thread* thread, int index, int* equal) {
    crs_Object* object = getIndex(thread, index);

    if (equal != NULL) {
        *equal = object->type == CRS_TYPE_BOOLEAN;
    }

    return crsO_test(object);
}

export crs_Integer crs_toIntegerX(crs_Thread* thread, int index, int* equal) {
    crs_Integer value;
    int         match = obj_cvtint(getIndex(thread, index), &value);

    if (equal != NULL) {
        *equal = match;
    }

    return value;
}

export crs_Float crs_toFloatX(crs_Thread* thread, int index, int* equal) {
    crs_Float value;
    int       match = obj_cvtfloat(getIndex(thread, index), &value);

    if (equal != NULL) {
        *equal = match;
    }

    return value;
}

export const char* crs_toStringX(crs_Thread* thread, int index, int* equal) {
    char* value;
    int   match = crsO_toString(getIndex(thread, index), &value);

    if (equal != NULL) {
        *equal = match;
    }

    return value;
}

export void* crs_toPointer(crs_Thread* thread, int index) {
    crs_Object* object = getIndex(thread, index);

    switch (object->type) {
        case CRS_TYPE_CFUNCTION:
            return *(void**)&obj_getc(object);
        case CRS_TYPE_STRING: case CRS_TYPE_TABLE:
        case CRS_TYPE_FUNCTION: case CRS_TYPE_THREAD:
            return obj_geth(object);
    }

    return NULL;
}

/*
 * ===========================
 *  push* functions
 * ===========================
 */

export void crs_pushNil(crs_Thread* thread) {
    crs_Object* object = adjustTop(thread, 1);
    obj_setn(object);
}

export void crs_pushBoolean(crs_Thread* thread, int value) {
    crs_Object* object = adjustTop(thread, 1);
    obj_setb(object, value != 0);
}

export void crs_pushInteger(crs_Thread* thread, crs_Integer value) {
    crs_Object* object = adjustTop(thread, 1);
    obj_seti(object, value);
}

export void crs_pushFloat(crs_Thread* thread, crs_Float value) {
    crs_Object* object = adjustTop(thread, 1);
    obj_setf(object, value);
}

export void crs_pushCFunction(crs_Thread* thread, crs_CFunction* function) {
    crs_Object* object = adjustTop(thread, 1);
    obj_setc(object, function);
}

export void crs_pushString(crs_Thread* thread, char* str) {
    crs_Object* object = adjustTop(thread, 1);
    crs_String* string = crsS_new(thread, str);

    obj_setgc(object, string);
    crsG_check(thread);
}

export void crs_pushTable(crs_Thread* thread) {
    crs_Object* object = adjustTop(thread, 1);
    crs_Table*  table  = crsT_new(thread);

    obj_setgc(object, table);
    crsG_check(thread);
}

export void crs_format(crs_Thread* thread, char* format, ...) {
    va_list args;

    va_start(args, format);
    crs_vformat(thread, format, args);
    va_end(args);
}

export void crs_vformat(crs_Thread* thread, char* format, va_list args) {
    crs_Object* object = adjustTop(thread, 1);
    crs_String* string = crsF_vformat(thread, format, args);

    obj_setgc(object, string);
    crsG_check(thread);
}

/*
 * ===========================
 *  functions
 * ===========================
 */

export void crs_call(crs_Thread* thread, int index, int args, int wanted) {
    crsV_call(thread, getIndex(thread, index), args, wanted);
    crsG_check(thread);
}

export int crs_pcall(crs_Thread* thread, int index, int args, int wanted) {
    int status = crsV_pcall(thread, getIndex(thread, index), args, wanted);

    if (status != CRS_OK) {
        crs_Object* object = adjustTop(thread, 1);
        obj_seto(object, &thread->error);
        obj_setn(&thread->error);
    }

    crsG_check(thread);
    return status;
}

static void* tryLoad(crs_Thread* thread, void* data) {
    UNUSED(thread);
    crs_Stream*   stream = data;
    crs_Function* func;
    crsR_fill(stream);

    if (stream->length && stream->buffer[0] == CRS_SIGNATURE[0]) {
        func = crsK_load(stream);
    } else {
        func = crsP_compile(stream);
    }

    return func;
}

export int crs_load(crs_Thread* thread, crs_Reader* reader, void* data,
                                        char* source) {
    crs_Function* func;
    crs_Stream    stream;
    crsR_init(thread, &stream, reader, data, source);

    size_t top    = call_savetop(thread);
    int    status = crsC_try(thread, &tryLoad, &stream, (void**)&func);
    call_restoretop(thread, top);
    crs_Object* result = adjustTop(thread, 1);

    switch (status) {
        case CRS_OK:
            obj_setgc(result, func);
            break;
        case CRS_CODEERR: {
            crs_String* string = crsD_loadError(thread, &stream);
            obj_setgc(result, string);
            break;
        }
        default:
            obj_seto(result, &thread->error);
    }

    obj_setn(&thread->error);

    crsG_check(thread);
    return status;
}

export int crs_dump(crs_Thread* thread, int index, crs_Writer* writer,
                                        void* data) {
    crs_Object* object = getIndex(thread, index);
    crs_Dump    dump;

    if (object->type != CRS_TYPE_FUNCTION) {
        crsC_errorf(thread, "cannot dump a %s value", crsO_name(object));
    }

    crsW_init(thread, &dump, writer, data);
    int status = crsK_dump(&dump, obj_getk(object));

    crsG_check(thread);
    return status;
}
