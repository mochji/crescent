#ifndef VM_H
#define VM_H

enum crescent_OpCode {
	OP_LOADC = 0,
	OP_LOADL = 1,

	OP_ADD   = 2,
	OP_SUB   = 3,
	OP_MUL   = 4,
	OP_POW   = 5,
	OP_DIV   = 6,
	OP_IDIV  = 7,
	OP_MOD   = 8,

	OP_BNOT  = 9,
	OP_BAND  = 10,
	OP_BNAND = 11,
	OP_BOR   = 12,
	OP_BNOR  = 13,
	OP_BXNOR = 14,
	OP_BXMOR = 15,
	OP_BSHL  = 16,
	OP_BSHR  = 17,

	OP_PRINT = 18,
	OP_EXIT  = 19
};

#define crescent_getOpCode(x) ((x >> 24) & 0xFF)
#define crescent_getOpA(x)    ((x >> 16) & 0xFF)
#define crescent_getOpB(x)    ((x >> 8)  & 0xFF)
#define crescent_getOpC(x)    ( x        & 0xFF)

void
crescent_executeBytecode(
	crescent_Stack* stack,
	uint32_t*       code
);

#endif
