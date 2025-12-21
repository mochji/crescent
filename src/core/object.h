/*
 * https://github.com/mochji/crescent
 * core/object.h
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#ifndef CRS_CORE_OBJECT_H
#define CRS_CORE_OBJECT_H

#include "conf.h"
#include "limit.h"

/*
 * the forward declaration here is temporary until i start development on the
 * garbage collector, which will create a "crs_GCHeader* struct that
 * points to the collectable type
 */

struct crs_String;
struct crs_Array;

union
crs_Value {
	int                b;
	crs_Integer        i;
	crs_Float          f;
	struct crs_String* s;
	struct crs_Array*  a;
	crs_CFunction*     c;
};

struct
crs_Object {
	crs_byte        type;
	union crs_Value value;
};

struct
crs_String {
	size_t size;
	size_t length;
	char*  value;
	size_t references;
};

struct
crs_Array {
	size_t             size;
	size_t             length;
	struct crs_Object* value;
	size_t             references;
};

typedef union  crs_Value  crs_Value;
typedef struct crs_Object crs_Object;
typedef struct crs_String crs_String;
typedef struct crs_Array  crs_Array;

extern int
crsO_compare(crs_Object* a, crs_Object* b);

extern int
crsO_clone(crs_Object* to, crs_Object* from);

extern int
crsO_deepClone(crs_Object* to, crs_Object* from);

extern void
crsO_free(crs_Object* object);

extern char*
crsO_typeName(int type);

extern int
crsO_toBoolean(crs_Object* object, int* match);

extern crs_Integer
crsO_toInteger(crs_Object* object, int* match);

extern crs_Float
crsO_toFloat(crs_Object* object, int* match);

extern char*
crsO_toString(crs_Object* object, int* match);

#define obj_cancall(type)   ((type) & 0x04)
#define obj_haslength(type) ((type) & 0x02)
#define obj_isnumber(type)  ((type) & 0x01)

#endif
