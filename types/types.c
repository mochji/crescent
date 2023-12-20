#include <stddef.h>

#include "../prefix.h"

#include "../state/state.h"

#define crescent_Byte            char
#define crescent_UnsignedByte    unsigned char

#define crescent_Integer         ssize_t
#define crescent_UnsignedInteger size_t

#ifdef CRESCENT_BITNESS == 32
	#define crescent_Float float
#else
	#define crescent_Float double
#endif

void crescent_pushByte(crescent_State* state, crescent_Byte value) {
	unsigned char data[1] = {crescent_bitCast(unsigned char, value)};

	crescent_stackPush(
		state->stack,
		data,
		1
	);
}

void crescent_pushUnsignedByte(crescent_State* state, crescent_UnsignedByte value) {
	unsigned char data[1] = {value};

	crescent_stackPush(
		state->stack,
		data,
		1
	);
}

void crescent_pushInteger(crescent_State* state, crescent_Integer value) {
	unsigned char* data;
	data = malloc(sizeof(crescent_Integer));

	if (!data)
		crescent_apiError("malloc failure!");

	for (int a = 0; a < sizeof(crescent_Integer); a++)
		data[a] = crescent_bitCast(unsigned char, (value >> (a * 8)) & 0xFF);

	crescent_stackPush(
		state->stack,
		data,
		sizeof(crescent_Integer)
	);
}

void crescent_pushUnsignedInteger(crescent_State* state, crescent_UnsignedInteger value) {
	unsigned char* data;
	data = malloc(sizeof(crescent_UnsignedInteger));

	if (!data)
		crescent_apiError("malloc failure!");

	for (int a = 0; a < sizeof(crescent_Integer); a++)
		data[a] = crescent_bitCast(unsigned char, (value >> (a * 8)) & 0xFF);

	crescent_stackPush(
		state->stack,
		data,
		sizeof(crescent_UnsignedInteger)
	);
}

void crescent_pushFloat(crescent_State* state, crescent_Float value) {
	unsigned char* data;
	data = malloc(sizeof(crescent_Float));

	if (!data)
		crescent_apiError("malloc failure!");

	for (int a = 0; a < sizeof(crescent_Float); a++)
		data[a] = crescent_bitCast(unsigned char, (value >> (a * 8)) & 0xFF);

	crescent_stackPush(
		state->stack,
		data,
		sizeof(crescent_Float)
	);
}
