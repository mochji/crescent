#include "stack.c"

/*
	Crescent bytecode

	Header

	\x27CRSCNT | Signature          | 7 bytes
	\x01       | Crescent version   | 1 byte
	\x04       | Instruction size   | 1 byte
	\x08       | size_t size        | 1 byte
	\x00       | Integral flag      | 1 byte
	\x???????? | Size of variables  | ? bytes
	\x???????? | Size of functions  | ? bytes
	\x???????? | Size of code       | ? bytes

	Variables
*/

typedef enum {
	// Operators

	// Math
	ADD = 0,
	SUB,
	MUL,
	POW,
	DIV,
	MOD,

	// Bitwise
	BAND,
	BOR,
	BXOR,
	BNOT,
	BXNOR,
	BLS,
	BRS,

	// Logical
	LAND,
	LOR,
	LXOR,
	LNOT,
	LXNOR,

	// Memory

	// Stack
	PUSH,
	POP
} crescent_Instruction;


