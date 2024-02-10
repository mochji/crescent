/*
 * https://github.com/mochji/crescent
 * vm/vm.h
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

#ifndef VM_VM_H
#define VM_VM_H

#include "conf.h"

enum
crescent_LogicalOp {
	LOP_NOT,
	LOP_AND,
	LOP_OR
};

enum
crescent_ArithOp {
	AOP_ADD,
	AOP_SUB,
	AOP_MUL,
	AOP_POW,
	AOP_DIV,
	AOP_MOD
};

enum
crescent_BitwiseOp {
	BOP_NOT,
	BOP_AND,
	BOP_NAND,
	BOP_OR,
	BOP_NOR,
	BOP_XOR,
	BOP_XNOR,
	BOP_BSL,
	BOP_BSR
};

typedef enum crescent_LogicalOp crescent_LogicalOp;
typedef enum crescent_ArithOp   crescent_ArithOp;
typedef enum crescent_BitwiseOp crescent_BitwiseOp;

#endif
