#ifndef TYPES_H
#define TYPES_H

#define crescent_Byte            char
#define crescent_UnsignedByte    unsigned char

#define crescent_Integer         ssize_t
#define crescent_UnsignedInteger size_t

#ifdef CRESCENT_BITNESS == 32
	#define crescent_Float float
#else
	#define crescent_Float double
#endif

void crescent_pushByte(crescent_State* state, crescent_Byte value);
void crescent_pushUnsignedByte(crescent_State* state, crescent_UnsignedByte value);
void crescent_pushInteger(crescent_State* state, crescent_Integer value);
void crescent_pushUnsignedInteger(crescent_State* state, crescent_UnsignedInteger value);
void crescent_pushFloat(crescent_State* state, crescent_Float value);

crescent_Byte crescent_popByte(crescent_State* state);
crescent_UnsignedByte crescent_popUnsignedByte(crescent_State* state);
crescent_Integer crescent_popInteger(crescent_State* state);
crescent_UnsignedInteger crescent_popUnsignedInteger(crescent_State* state);
crescent_Float crescent_popFloat(crescent_State* state);

#endif
