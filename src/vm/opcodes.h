#ifndef OPCODES_H
#define OPCODES_H

/*
	VM Instruction Formats

	i = OpCode

	A = operand A
	B = operand B
	C = operand C

	x = Extra 8 bits for last operand s = Next operand is signed

			DDDDDDDD CCCCCCCC DDDDDDDD AAAAAAAA
	iABC   |    OpC |    OpB |    OpA | OpCode |
	iABx   |    unsigned OpB |    OpA | OpCode |
	iAsBx  |      signed OpB |    OpA | OpCode |
	iAxx   |             unsigned OpA | OpCode |
	isAxx  |               signed OpA | OpCode |
*/

enum OpMode {
	iABC  = 0,
	iABx  = 1,
	iAsBx = 2,
	iAxx  = 3,
	isAxx = 4
};

enum OpCode {
	OP_LOADC = 0,
	OP_LOADL = 1,

	OP_ADD   = 2,
	OP_SUB   = 3,
	OP_MUL   = 4,
	OP_POW   = 5,
	OP_DIV   = 6,
	OP_MOD   = 8,

	OP_BNOT  = 9,
	OP_BAND  = 10,
	OP_BNAND = 11,
	OP_BOR   = 12,
	OP_BNOR  = 13,
	OP_BXOR  = 14,
	OP_BXNOR = 15,
	OP_BSHL  = 16,
	OP_BSHR  = 17,

	OP_JE    = 18,
	OP_JNE   = 19,
	OP_JG    = 20,
	OP_JL    = 21,
	OP_JGE   = 22,
	OP_JLE   = 23,
	OP_JZ    = 24,
	OP_JNZ   = 25
};

#define crescentVM_iABC(instruction)  \
	opCode   = ((x >> 24) & 0xFF);    \
	operandA = ((x >> 16) & 0xFF);    \
	operandB = ((x >> 8)  & 0xFF);    \
	operandC = ( x        & 0xFF)

#define crescentVM_iABx(instruction)  \
	opCode   = ((x >> 24) & 0xFF);    \
	operandA = ((x >> 16) & 0xFF);    \
	operandB = ( x        & 0xFFFF)

#define crescentVM_iAsBx(instruction) \
	opCode   = ((x >> 24) & 0xFF);    \
	operandA = ((x >> 16) & 0xFF);    \
	operandB = ( x        & 0xFFFF)

#define crescentVM_iAxx(instruction)  \
	opCode   = ((x >> 24) & 0xFF);    \
	operandA = ( x >> 16  & 0xFFFF)   \

#define crescentVM_isAxx(instruction) \
	opCode   = ((x >> 24) & 0xFF);    \
	operandA = ( x >> 16  & 0xFFFF)

#endif
