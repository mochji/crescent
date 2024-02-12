/*
 * https://github.com/mochji/crescent
 * vm/opcodes.c
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

#include "conf.h"

#include "vm/opcodes.h"

crescentVM_OpMode
crescentVM_mode[] = {
	iAB,  /* OP_MOV */

	iABx, /* OP_LODPI */
	iABx, /* OP_LODNI */
	iA,   /* OP_LODN */
	iA,   /* OP_LODT */
	iA,   /* OP_LODF */
	iABx, /* OP_LODC */
	iABx, /* OP_LODCX */

	iABC, /* OP_ADD */
	iABC, /* OP_SUB */
	iABC, /* OP_MUL */
	iABC, /* OP_POW */
	iABC, /* OP_DIV */
	iABC, /* OP_MOD */

	iA,   /* OP_INC */
	iA,   /* OP_DEC */

	iAB,  /* OP_BNOT */
	iABC, /* OP_BAND */
	iABC, /* OP_BNAND */
	iABC, /* OP_BOR */
	iABC, /* OP_BNOR */
	iABC, /* OP_BXOR */
	iABC, /* OP_BXNOR */
	iABC, /* OP_BSL */
	iABC, /* OP_BSR */

	iABC, /* OP_IS */
	iABC, /* OP_EQ */
	iABC, /* OP_NE */
	iABC, /* OP_GT */
	iABC, /* OP_GE */
	iABC, /* OP_LT */
	iABC, /* OP_LE */

	iABC, /* OP_CALL */

	iAxx, /* OP_JMP */
	iAxx, /* OP_JMPX */
	iAxx, /* OP_JMPF */
	iAxx, /* OP_JMPB */

	iA,   /* OP_JIS */
	iAB,  /* OP_JE */
	iAB,  /* OP_JNE */
	iAB,  /* OP_JG */
	iAB,  /* OP_JGE */
	iAB,  /* OP_JL */
	iAB   /* OP_JLE */
};

#endif
