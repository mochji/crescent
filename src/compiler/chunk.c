/*
 * https://github.com/mochji/crescent
 * compiler/chunk.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <limits.h>

#include "crescent/conf.h"
#include "limit.h"

#include "types/table.h"
#include "types/function.h"
#include "core/state.h"
#include "core/memory.h"
#include "core/format.h"
#include "vm/opcodes.h"

#include "compiler/chunk.h"

noret crsI_error(Chunk* chunk, char* format, ...) {
    crs_Thread* thread = chunk->thread;
    crs_String* error;
    va_list     args;

    va_start(args, format);
    error = crsF_vformat(thread, format, args);
    va_end(args);

    obj_setgc(&thread->error, error);
    crsC_throw(thread, CRS_STATUS_CODEERR);
}

static void limitError(Chunk* chunk, unsigned max, char* what) {
	crsI_error(chunk, "too many %s (limit is %u)", what, max);
}

static void data_init(Data* data, void** vec, unsigned max, size_t type) {
	max = max > SIZE_MAX / type
		? SIZE_MAX / type
		: max;

	data->vec   = vec;
	data->count = 0;
	data->max   = max;
	data->type  = type;
}

static void data_resize(Chunk* chunk, Data* data, unsigned size) {
	void* vec = mem_realloc(chunk->thread, *data->vec,
		size * data->type, *data->size * data->type);

	if (vec == NULL) {
		crsM_error(chunk->thread);
	}

	*data->vec  = vec;
	*data->size = size;
}

/* ensure space for at least one more element */
static unsigned data_check(Chunk* chunk, Data* data, char* what) {
	unsigned size = *data->size;
	unsigned max  = data->max;

	if (data->count < size) {
		return data->count++;
	}

	if (size == max) {
		limitError(chunk, max, what);
	} else if (size > max / 2) {
		size = max;
	} else {
		size *= 2;
	}

	data_resize(chunk, data, size);
    return data->count++;
}

/* finalize a function vector */
static void data_shrink(Chunk* chunk, Data* data) {
	if (data->count) {
		data_resize(chunk, data, data->count);
	} else {
		mem_dealloc(chunk->thread, *data->vec, *data->size * data->type);
		*data->vec  = NULL;
		*data->size = 0;
	}
}

static void data_free(Chunk* chunk, Data* data) {
    mem_dealloc(chunk->thread, *data->vec, *data->size * data->type);
}

/*
 * ===========================
 *  chunk
 * ===========================
 */

crs_Function* crsI_newChunk(Chunk* chunk, crs_Thread* thread, Parser* parser) {
    crs_Table*    cTable = crsT_new(thread);
    crs_Function* func;

    crsC_checkFree(thread, 1, 1);
    crsC_anchor(thread, obj_toheader(cTable));
    func = crsK_new(thread, 32, 8, 8);
    /* function anchored by caller (to parent func or stack) */

    chunk->thread  = thread;
    chunk->parser  = parser;
    chunk->scope   = NULL;
    chunk->regs    = 0;
    chunk->fVar    = parser->vars.count;
    chunk->fLabel  = parser->labels.count;
    chunk->fGoto   = parser->gotos.count;
    chunk->func    = func;
    chunk->cTable  = cTable;
    data_init(&chunk->code, (void**)&func->code,
        UINT_MAX, sizeof(crs_instr));
    data_init(&chunk->consts, (void**)&func->consts,
        MAX_Bx, sizeof(crs_Object));
    data_init(&chunk->nested, (void**)&func->nested,
        MAX_Bx, sizeof(crs_Function*));

    return func;
}

/* finish compiling a function */
crs_Function* crsI_finish(Chunk* chunk) {
    data_shrink(chunk, &chunk->code);
    data_shrink(chunk, &chunk->consts);
    data_shrink(chunk, &chunk->nested);

    return chunk->func;
}

unsigned crsI_nested(Chunk* chunk, crs_Function* func) {
    Data*    nested = &chunk->nested;
    unsigned index  = data_check(chunk, nested, "nested functions");

    chunk->func->nested[index] = func;
    return index;
}

/*
 * ===========================
 *  code
 * ===========================
 */

unsigned crsI_emit(Chunk* chunk, crs_instr i) {
    Data*    code = &chunk->code;
    unsigned pc   = data_check(chunk, code, "instructions");

    chunk->func->code[pc] = i;
    return pc;
}

/*
 * ===========================
 *  constants
 * ===========================
 */

unsigned crsI_newConst(Chunk* chunk, crs_Object* value) {
    Data*    consts = &chunk->consts;
    unsigned index  = data_check(chunk, consts, "constants");

    obj_seto(&chunk->func->consts[index], value);
    return index;
}
