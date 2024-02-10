/*
 * https://github.com/mochji/crescent
 * vm/opcodes.h
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VM_OPCODES_H
#define VM_OPCODES_H

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
 * Why are there no OpModes that have an operand that is signed? The
 * instruction set doesn't need it. For stuff like indexes, that's obvious,
 * but what about loading values? Instead of having 2 OpCodes for load
 * unsigned integer and load signed integer, you can have still 2 OpCodes
 * but hafve them load positive integer and load negative integer, allowing
 * for a higher range of values. What about jumping locally? Jump forwards and
 * jump backwards. Is this kinda stupid? yeah. Is it efficient? kinda.
 */

enum
crescentVM_OpMode {
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
 */

enum
crescentVM_OpCode {
/*  enum         operation                mode          */
	OP_MOV,   /* S[A] = S[B]              iAB           */

	OP_LODPI, /* S[A] = B                 iABx          */
	OP_LODNI, /* S[A] = -B                iABx          */
	OP_LODN,  /* S[A] = nil               iA            */
	OP_LODT,  /* S[A] = true              iA            */
	OP_LODF,  /* S[A] = false             iA            */
	OP_LODC,  /* S[A} = C[B]              iABx          */
	OP_LODCX, /* S[A} = CE[B]             iABx          */

	OP_ADD,   /* S[A] = S[B] + S[C]       iABC          */
	OP_SUB,   /* S[A] = S[B] - S[C]       iABC          */
	OP_MUL,   /* S[A} = S[B} * S[C]       iABC          */
	OP_POW,   /* S[A] = S[B] ^ S[C]       iABC          */
	OP_DIV,   /* S[A] = S[B] / S[C]       iABC          */
	OP_MOD,   /* S[A} = S[B] % S[C]       iABC          */

	OP_INC,   /* S[A] += 1                iA            */
	OP_DEC,   /* S[A] -= 1                iA            */

	OP_BNOT,  /* S[A] = !S[B]             iAB           */
	OP_BAND,  /* S[A] = S[B] & S[C]       iABC          */
	OP_BNAND, /* S[A] = !(S[B] & S[C])    iABC          */
	OP_BOR,   /* S[A] = S[B] | S[C]       iABC          */
	OP_BNOR,  /* S[A] = !(S[B] | S[C])    iABC          */
	OP_BXOR,  /* S[A] = S[B] ^ S[C]       iABC          */
	OP_BXNOR, /* S[A] = !(S[B] ^ S[C])    iABC          */
	OP_BSL,   /* S[A] = S[A] << S[C]      iABC          */
	OP_BSR,   /* S[A] = S[A] >> S[C]      iABC          */

	OP_CALL,  /* S[A}()                   iA            */

	OP_JMP,   /* PC = L[A]                iAxx          */
	OP_JMPX,  /* PC = LE[A}               iAxx          */
	OP_JMPF,  /* PC += A                  iAxx          */
	OP_JMPB,  /* PC -= A                  iAxx          */

	OP_JE,    /* if S[A] == S[A] PC++     iAB           */
	OP_JNE,   /* if S[A] != S[A} PC++     iAB           */
	OP_JZ,    /* if S[A] == 0 PC++        iA            */
	OP_JNZ,   /* if S[A} != 0 PC++        iA            */
	OP_JG,    /* if S[A] > S[B] PC++      iAB           */
	OP_JGE,   /* if S[A} >= S[B] PC++     iAB           */
	OP_JL,    /* if S[A} < S[B] PC++      iAB           */
	OP_JLE    /* if S[A] <= S[B] PC++     iAB           */
};

typedef enum crescentVM_OpMode crescentVM_OpMode;
typedef enum crescentVM_OpCode crescentVM_OpCode;

#endif
