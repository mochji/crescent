/*
 * https://github.com/mochji/crescent
 * types/function.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/memory.h"
#include "core/buffer.h"

#include "types/function.h"

void
crsK_free(crs_Thread* thread, crs_Function* function) {
	mem_vfree(thread, function->code, function->nCode);
	mem_vfree(thread, function->constants, function->nConstants);
	mem_vfree(thread, function->nested, function->nNested);
	mem_free(thread, function);
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
