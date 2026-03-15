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
 *       | AAAAAAAA | BBBBBBBB | CCCCCCCC | DDDDDDDD |
 * iABC  | OpCode   | Op. A    | Op. B    | Op. C    |
 * iABx  | OpCode   | Op. A    | Op. B               |
 * iAsBx | OpCode   | Op. A    | Op. B (signed)      |
 * iAxx  | OpCode   | Op. A                          |
 * isAxx | OpCode   | Op. A (signed)                 |
 *
 * - i: OpCode
 * - A: Operand A
 * - B: Operand B
 * - C: Operand C
 * - x: Extended (+8 bits)
 * - s: Signed
 */

enum
crs_OpMode {
	iABC,
	iABx,
	iAsBx,
	iAxx,
	isAxx
};

/*
 * S = stack
 * C = constants
 */

enum
crs_OpCode {
/*  enum            operation                mode        notes */
	OP_MOV,      /* S[A] = S[B]              iABC              */

	OP_LODN,     /* S[A] = nil               iABC              */
	OP_LODT,     /* S[A] = true              iABC              */
	OP_LODF,     /* S[A] = false             iABC              */
	OP_LODI,     /* S[A] = sBx               iAsBx             */
	OP_LODC,     /* S[A] = C[B]              iABx              */

	OP_ADD,      /* S[A] = S[B] + S[C]       iABC              */
	OP_SUB,      /* S[A] = S[B] - S[C]       iABC              */
	OP_MUL,      /* S[A] = S[B] * S[C]       iABC              */
	OP_DIV,      /* S[A] = S[B] / S[C]       iABC              */
	OP_POW,      /* S[A] = S[B] ^ S[C]       iABC              */
	OP_MOD,      /* S[A] = S[B] % S[C]       iABC              */

	OP_NOT,      /* S[A] = not S[B]          iABC              */
	OP_AND,      /* S[A] = S[B] and S[C]     iABC              */
	OP_OR,       /* S[A] = S[B] or S[C]      iABC              */

	OP_BNOT,     /* S[A] = ~S[B]             iABC              */
	OP_BAND,     /* S[A] = S[B] & S[C]       iABC              */
	OP_BOR,      /* S[A] = S[B] | S[C]       iABC              */
	OP_BXOR,     /* S[A] = S[B] ^ S[C]       iABC              */

	OP_EQ,       /* S[A] = S[B] == S[C]      iABC              */
	OP_LT,       /* S[A] = S[B] < S[C]       iABC              */
	OP_LE,       /* S[A] = S[B] <= S[C]      iABC              */

	OP_LENGTH,   /* S[A] = #S[B]             iABC              */
	OP_CONCAT,   /* S[A] = S[B] .. S[C]      iABC              */
	OP_GET,      /* S[A] = S[B][S[C]]        iABC              */
	OP_SET,      /* S[B][S[C]] = S[A]        iABC              */

	OP_CALL,     /* S[A] S[B]() (C args)     iABC              */
	OP_RETURN0,  /* return                   iABC              */
	OP_RETURN1,  /* return S[A]              iABC              */
	OP_RETURN,   /* return S[A] .. S[B]      iABC              */

	OP_TEST,     /* if S[A] then PC++        iABC              */
	OP_JMP       /* PC += sAxx               isAxx             */
};

typedef enum crs_OpMode crs_OpMode;
typedef enum crs_OpCode crs_OpCode;

extern crs_OpMode  crsV_mode[];
extern const char* crsV_name[];

#endif
