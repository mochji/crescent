#include <stdint.h>

typedef enum {
	iABC = 0,
	iABx = 1,
	iAxx = 2
} OpMode;

typedef enum {
	OP_LOADC = 0,
	OP_LOADL = 1,

	OP_ADD   = 2,
	OP_SUB   = 3,
	OP_MUL   = 4,
	OP_POW   = 5,
	OP_DIV   = 6,
	OP_MOD   = 7,

	OP_BNOT  = 8,
	OP_BAND  = 9,
	OP_BNAND = 10,
	OP_BXAND = 11,
	OP_BOR   = 12,
	OP_BNOR  = 13,
	OP_BXOR  = 14,
	OP_BXNOR = 15,

	OP_JMP   = 16,
	OP_JE    = 17,
	OP_JNE   = 18,
	OP_JG    = 19,
	OP_JL    = 20,
	OP_JGE   = 21,
	OP_JLE   = 22,
	OP_JZ    = 23,
	OP_JNZ   = 24,

	OP_EXIT  = 63
} OpCode;

#define CRESCENT_VM_GETOPCODE(x)     ( x            & 0xFC)
#define CRESCENT_VM_GETMOD(x)        ( x            & 0x03)

#define CRESCENT_VM_GET8(x, offset)  ((x << offset) & 0xFF)
#define CRESCENT_VM_GET16(x, offset) ((x << offset) & 0xFFFF)
#define CRESCENT_VM_GET24(x, offset) ((x << offset) & 0xFFFFFF)

void crescentVM_execute(crescent_State* state, uint32_t code) {
	uint32_t instruction;

	unsigned char opCode;
	unsigned char mod;

	uint32_t operandA;
	uint32_t operandB;
	uint32_t operandC;

	size_t a = 0;

	for (;;) {
		instruction = CRESCENT_VM_GETOPCODE(code[a++]);

		opCode = CRESCENT_VM_GETOPCODE(instruction);
		mod    = CRESCENT_VM_GETMOD(instruction);

		switch (opCode) {
			case OP_LOADC:
				a
		}
	}
}
