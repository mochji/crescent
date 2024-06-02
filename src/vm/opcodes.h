/*
 * https://github.com/mochji/crescent
 * vm/opcodes.h
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * MIT License
 */

#ifndef CRESCENT_VM_OPCODES_H
#define CRESCENT_VM_OPCODES_H

#include "conf.h"

/*
 * Crescent VM instruction formats
 *
 * An instruction can be anywhere from 8 to 32 bits wide, provided that it is
 * a multiple of 8. (8, 16, 24, 32)
 *
 *       | DDDDDDDD | CCCCCCCC | BBBBBBBB | AAAAAAAA |
 * iABC  | Op. C    | Op B.    | Op. A    | OpCode   | (32 bits)
 * iABx  | Op. B               | Op. A    | OpCode   | (32 bits)
 * iAxx  | Op. A                          | OpCode   | (32 bits)
 * iAB              | Op. B    | Op. A    | OpCode   | (24 bits)
 * iA                          | Op. A    | OpCode   | (16 bits)
 * i                                      | OpCode   | (8  bits)
 *
 * - i: OpCode
 * - A: Operand A
 * - B: Operand B
 * - C: Operand C
 * - x: Extended (+8 bits)
 *
 * Why are there no signed operands?
 *
 * The instruction set doesn't need it, we can have 2 instructions that say
 * "load positive integer" and "load negative integer" which allows a greater
 * range of values and simplifies the instruction set. There also won't be an
 * issue where we run out of OpCodes, since an OpCode is 8 bits which allows
 * for 256 possible values, more than enough for the Crescent VM.
 */

enum
crescentV_OpMode {
	iABC,
	iABx,
	iAxx,
	iAB,
	iA,
	i
};

/*
 * S  = stack
 * C  = constants
 * CE = constants + 0xFFFF
 * L  = labels
 * LE = labels + 0xFFFFFF
 * G  = globals
 */

enum
crescentV_OpCode {
/*  enum            operation                mode        notes */
	OP_MOV,      /* S[A] = S[B]              iAB               */

	OP_LODN,     /* S[A] = nil               iA                */
	OP_LODT,     /* S[A] = true              iA                */
	OP_LODF,     /* S[A] = false             iA                */
	OP_MODM,     /* S[A] = maybe             iA                */
	OP_LODPI,    /* S[A] = B                 iABx              */
	OP_LODNI,    /* S[A] = -B                iABx              */
	OP_LODS,     /* S[A] = "" (string)       iA                */
	OP_LODA,     /* S[A] = [] (array)        iA                */
	OP_LODC,     /* S[A] = C[B]              iABx              */
	OP_LODCX,    /* S[A] = CE[B]             iABx              */

	OP_GETG,     /* S[A] = G[S{A]]           iAB               */
	OP_SETG,     /* G[S[A]] = S[B]           iAB               */

	OP_ADD,      /* S[A] = S[B] + S[C]       iABC              */
	OP_SUB,      /* S[A] = S[B] - S[C]       iABC              */
	OP_MUL,      /* S[A] = S[B] * S[C]       iABC              */
	OP_POW,      /* S[A] = S[B] ^ S[C]       iABC              */
	OP_DIV,      /* S[A] = S[B] / S[C]       iABC              */
	OP_MOD,      /* S[A] = S[B] % S[C]       iABC              */

	OP_INC,      /* S[A] += 1                iA                */
	OP_DEC,      /* S[A] -= 1                iA                */

	OP_BNOT,     /* S[A] = !S[B]             iAB               */
	OP_BAND,     /* S[A] = S[B] & S[C]       iABC              */
	OP_BNAND,    /* S[A] = !(S[B] & S[C])    iABC              */
	OP_BOR,      /* S[A] = S[B] | S[C]       iABC              */
	OP_BNOR,     /* S[A] = !(S[B] | S[C])    iABC              */
	OP_BXOR,     /* S[A] = S[B] ^ S[C]       iABC              */
	OP_BXNOR,    /* S[A] = !(S[B] ^ S[C])    iABC              */
	OP_BSL,      /* S[A] = S[A] << S[C]      iABC              */
	OP_BSR,      /* S[A] = S[A] >> S[C]      iABC              */

	OP_EVAL,     /* S[A] = (boolean)S[B]     iAB               */
	OP_EQ,       /* S[A] = S[B] == S[C]      iABC              */
	OP_NE,       /* S[A] = S[B] != S[C]      iABC              */
	OP_GT,       /* S[A] = S[B] > S[C]       iABC              */
	OP_GE,       /* S[A] = S[B] >= S[C]      iABC              */
	OP_LT,       /* S[A] = S[B] < S[C]       iABC              */
	OP_LE,       /* S[A] = S[B] <= S[C]      iABC              */

	OP_IS,       /* S[A] = &S[B] == &S[C]    iABC        0     */

	OP_LENGTH,   /* S[A] = #S[B]             iABC              */
	OP_CONCAT,   /* S[A] = S[A] .. S[B]      iAB               */
	OP_GET,      /* S[A] = S[B][S[C]]        iABC              */
	OP_SET,      /* S[A][S[B]] = S[C]        iABC              */

	OP_CALL,     /* S[A] S[B]() (C args)     iABC              */
	OP_TCALL,    /* S[A] S[B]() (C args)     iABC              */
	OP_RETURN0,  /* return                   i                 */
	OP_RETURN1,  /* return S[A]              iA                */
	OP_RETURN,   /* return S[A] .. S[B]      iAB               */

	OP_JMPF,     /* PC += A                  iAxx              */
	OP_JMPB,     /* PC -= A                  iAxx              */

	OP_JEVAL,    /* if (boolean)S[A] PC++    iA                */
	OP_JE,       /* if S[A] == S[A] PC++     iAB               */
	OP_JNE,      /* if S[A] != S[A] PC++     iAB               */
	OP_JG,       /* if S[A] > S[B] PC++      iAB               */
	OP_JGE,      /* if S[A] >= S[B] PC++     iAB               */
	OP_JL,       /* if S[A] < S[B] PC++      iAB               */
	OP_JLE       /* if S[A] <= S[B] PC++     iAB               */
};

/*
 * Notes:
 *
 * 0: What the operation section for the OP_IS OpCode says is wrong, basically
 *    it compares the address of complex types. It will always return false for
 *    objects that are different types or simple. Otherwise it will compare the
 *    address of the actual data (value.s for strings, value.a for arrays,
 *    etc).
 */

typedef enum crescentV_OpMode crescentV_OpMode;
typedef enum crescentV_OpCode crescentV_OpCode;

extern crescentV_OpMode
crescentV_mode[];

#endif
