/*
 * https://github.com/mochji/crescent
 * types/function.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/memory.h"
#include "core/buffer.h"
#include "core/gc.h"

#include "types/function.h"

crs_Function*
crsK_new(crs_Thread* thread, crs_u32 nCode, crs_u32 nConstants, crs_u32 nNested) {
	crs_Function* func = mem_new(thread, crs_Function);

	if (func == NULL) {
		crsM_error(thread);
	}

	crs_instr*     code      = mem_vnew(thread, nCode, crs_instr);
	crs_Object*    constants = mem_vnew(thread, nConstants, crs_Object);
	crs_Function** nested    = mem_vnew(thread, nNested, crs_Function*);

	if (code == NULL || constants == NULL || nested == NULL) {
		mem_vfree(thread, code, nCode);
		mem_vfree(thread, constants, nConstants);
		mem_vfree(thread, nested, nNested);
		crsM_error(thread);
	}

	func->nCode      = nCode;
	func->nConstants = nConstants;
	func->nNested    = nNested;

	for (crs_u32 i = 0; i < nConstants; i++) {
		obj_setn(&constants[i]);
	}

	for (crs_u32 i = 0; i < nNested; i++) {
		nested[i] = NULL;
	}

	return crsG_add(thread, func, CRS_TYPE_FUNCTION);
}

void
crsK_free(crs_Thread* thread, crs_Function* func) {
	mem_vfree(thread, func->code, func->nCode);
	mem_vfree(thread, func->constants, func->nConstants);
	mem_vfree(thread, func->nested, func->nNested);
	mem_free(thread, func);
}

/*
 * Bytecode dump format
 *
 * WARNING: Bytecode dumps are not portable across different versions and
 *          configurations of Crescent. You should not distribute code via
 *          dumps.
 *
 * global header {
 *   char[4] | signature
 *   char[8] | decoding check
 *   byte    | crescent version
 *   byte    | endianness (0 = big endian; 1 = little endian)
 *   byte    | sizeof(crs_u32)
 *   byte    | sizeof(crs_Integer)
 *   byte    | sizeof(crs_Float)
 * }
 *
 * function {
 *   byte | flags
 *   byte | args
 *   byte | top
 *   u32  | # of instructions
 *   u32  | # of constants
 *   u32  | # of nested functions
 *
 *   crs_instr[# of instructions]
 *   constant[# of constants]
 *   function[# of nested functions]
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
