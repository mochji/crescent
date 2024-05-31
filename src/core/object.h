/*
 * https://github.com/mochji/crescent
 * core/object.h
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * MIT License
 */

#ifndef CRESCENT_CORE_OBJECT_H
#define CRESCENT_CORE_OBJECT_H

#include "conf.h"

struct crescent_String;
struct crescent_Array;

union
crescent_Value {
	int                     b;
	crescent_Integer        i;
	crescent_Float          f;
	struct crescent_String* s;
	struct crescent_Array*  a;
	crescent_CFunction*     c;
};

struct
crescent_Object {
	unsigned char        type;
	union crescent_Value value;
};

typedef union  crescent_Value  crescent_Value;
typedef struct crescent_Object crescent_Object;

extern int
crescentO_compare(crescent_Object* a, crescent_Object* b);

extern int
crescentO_clone(crescent_Object* to, crescent_Object* from);

extern int
crescentO_deepClone(crescent_Object* to, crescent_Object* from);

extern void
crescentO_free(crescent_Object* object);

extern char*
crescentO_typeName(int type);

extern int
crescentO_toBoolean(crescent_Object* object, int* isBoolean);

extern crescent_Integer
crescentO_toInteger(crescent_Object* object, int* isInteger);

extern crescent_Float
crescentO_toFloat(crescent_Object* object, int* isFloat);

extern char*
crescentO_toString(crescent_Object* object, int* isString, int* addToGC);

extern crescent_CFunction*
crescentO_toCFunction(crescent_Object* object, int* isCFunction);

#define obj_haslength(type) ((type) & 0x04)
#define obj_cancall(type)   ((type) & 0x02)
#define obj_isnumber(type)  ((type) & 0x01)

#endif
