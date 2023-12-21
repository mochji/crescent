#include <stdint.h>

#include "prefix.h"

#include "stack.h"

enum crescent_OpCode {
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

	OP_EXIT  = 19
};

#define crescent_getOpCode(x) ((x >> 24) & 0xFF)
#define crescent_getOpA(x)    ((x >> 16) & 0xFF)
#define crescent_getOpB(x)    ((x >> 8)  & 0xFF)
#define crescent_getOpC(x)    ( x        & 0xFF)

void crescent_executeBytecode(crescent_Stack* stack, uint32_t* code) {
	int64_t registers[3] = {};

	size_t   programCounter = 0;
	uint32_t instruction;

	while (1) {
		instruction = code[programCounter];

		unsigned char opCode   = crescent_getOpCode(instruction);
		unsigned char opA      = crescent_getOpA(instruction);
		unsigned char opB      = crescent_getOpB(instruction);
		unsigned char opC      = crescent_getOpC(instruction);

		switch (opCode) {
			case (OP_LOADC):
				registers[opA] = opB;

				break;
			case (OP_LOADL):
				registers[opA] = stack->data[opB];

				break;
			case (OP_ADD):
				registers[opA] = registers[opB] + registers[opC];

				break;
			case (OP_SUB):
				registers[opA] = registers[opB] - registers[opC];

				break;
			case (OP_MUL):
				registers[opA] = registers[opB] * registers[opC];

				break;
			case (OP_POW):
				int64_t base     = registers[opB];
				int64_t exponent = registers[opC];

				int64_t result = 1;

				for (uint64_t a = 0; a < exponent;)
					base *= base;

				registers[opA] = result;
			case (OP_DIV):
				registers[opA] = registers[opB] / registers[opC];

				break;
			case (OP_MOD):
				registers[opA] = registers[opB] % registers[opC];

				break;
			case (OP_BNOT):
				registers[opA] = ~registers[opB];

				break;
			case (OP_BAND):
				registers[opA] = registers[opB] & registers[opC];

				break;
			case (OP_BNAND):
				registers[opA] = ~(registers[opB] & registers[opC]);

				break;
			case (OP_BOR):
				registers[opA] = registers[opB] | registers[opC];

				break;
			case (OP_BNOR):
				registers[opA] = ~(registers[opB] | registers[opC]);

				break;
			case (OP_BXOR):
				registers[opA] = registers[opB] ^ registers[opC];

				break;
			case (OP_BXNOR):
				registers[opA] = ~(registers[opB]^registers[opC]);

				break;
			case (OP_BSHL):
				registers[opA] = registers[opB] << registers[opC];

				break;
			case (OP_BSHR):
				registers[opA] = registers[opB] >> registers[opC];

				break;
			case (OP_EXIT):
				exit(0);

				break;
			default:
				printf("temporary error message!\nopCode: %d\npc: %d\n", opCode, programCounter);

				exit(69);
		}

		programCounter++;
	}
}
