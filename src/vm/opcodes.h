/*
 * https://github.com/mochji/crescent
 * vm/opcodes.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_VM_OPCODES_H
#define CRS_VM_OPCODES_H

#include "conf.h"
#include "limit.h"

/*
 * Crescent VM instruction formats
 *
 * An instruction is 32 bits wide.
 *
 *       | DDDDDDDD | CCCCCCCC | BBBBBBBB | AAAAAAAA |
 * iABC  | Op. C    | Op. B    | Op. A    | OpCode   |
 * iABx  | Op. B               | Op. A    | OpCode   |
 * iAsBx | Op. B (signed)      | Op. A    | OpCode   |
 * iAxx  | Op. A                          | OpCode   |
 * isAxx | Op. A (signed)                 | OpCode   |
 *
 * - i: OpCode
 * - A: Operand A
 * - B: Operand B
 * - C: Operand C
 * - x: Extended (+8 bits)
 * - s: Signed
 */

typedef enum {
	iABC,
	iABx,
	iAsBx,
	iAxx,
	isAxx
} crs_OpMode;

/*
 * S = stack
 * C = constants
 */

/* order OPCODE */
typedef enum {
/*  enum            operation                mode        notes */
	OP_MOV,      /* S[A] = S[B]              iABC              */

	OP_LODN,     /* S[A] = nil               iABC              */
	OP_LODT,     /* S[A] = true              iABC              */
	OP_LODF,     /* S[A] = false             iABC              */
	OP_LODI,     /* S[A] = sBx               iAsBx             */
	OP_LODC,     /* S[A] = C[B]              iABx              */

	OP_UNM,      /* S[A] = -S[B]             iABC              */
	OP_ADD,      /* S[A] = S[B] + S[C]       iABC              */
	OP_SUB,      /* S[A] = S[B] - S[C]       iABC              */
	OP_MUL,      /* S[A] = S[B] * S[C]       iABC              */
	OP_DIV,      /* S[A] = S[B] / S[C]       iABC              */
	OP_POW,      /* S[A] = S[B] ^ S[C]       iABC              */
	OP_MOD,      /* S[A] = S[B] % S[C]       iABC              */

	OP_NOT,      /* S[A] = !S[B]             iABC              */

	OP_BNOT,     /* S[A] = ~S[B]             iABC              */
	OP_BAND,     /* S[A] = S[B] & S[C]       iABC              */
	OP_BOR,      /* S[A] = S[B] | S[C]       iABC              */
	OP_BXOR,     /* S[A] = S[B] ~ S[C]       iABC              */

	OP_EQ,       /* S[A] = S[B] == S[C]      iABC              */
	OP_LT,       /* S[A] = S[B] < S[C]       iABC              */
	OP_LE,       /* S[A] = S[B] <= S[C]      iABC              */

	OP_LENGTH,   /* S[A] = #S[B]             iABC              */
	OP_CONCAT,   /* S[A] = S[B] .. S[C]      iABC              */
	OP_GET,      /* S[A] = S[B][S[C]]        iABC              */
	OP_SET,      /* S[B][S[C]] = S[A]        iABC              */

	OP_CALL,     /* S[A] S[B](S[B + 1]) (#C) iABC              */
	OP_RETURN,   /* return S[B-A+1] .. S[B]  iABC              */

	OP_TEST,     /* if S[A] then PC++        iABC              */
	OP_JMP       /* PC += sAxx               isAxx             */
} crs_OpCode;

extern crs_OpMode  crsV_mode[];
extern const char* crsV_name[];

#endif
