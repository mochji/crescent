/*
 * https://github.com/mochji/crescent
 * core/object.h
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#ifndef CRS_CORE_OBJECT_H
#define CRS_CORE_OBJECT_H

#include <stddef.h>

#include "conf.h"
#include "limit.h"

/*
 * All collectable objects start with crs_GCHeader, which is how they are
 * referenced in the collector. To convert a header to an object (and vice
 * versa), you can just cast it. The header is always at the beginning, meaning
 * that it has an offset of zero bytes and the same pointer points to both the
 * header and type.
 *
 * (see obj_to* macros after the function definitions)
 */

struct
crs_GCHeader {
	struct crs_GCHeader* next;
	struct crs_GCHeader* set;
	crs_byte             mark;
	crs_byte             type;
};

union
crs_Value {
	int                  b;
	crs_Integer          i;
	crs_Float            f;
	crs_CFunction*       c;
	struct crs_GCHeader* gc;
};

struct
crs_Object {
	crs_byte        type;
	union crs_Value value;
};

typedef struct crs_GCHeader crs_GCHeader;
typedef union  crs_Value    crs_Value;
typedef struct crs_Object   crs_Object;

struct
crs_String {
	struct crs_GCHeader header;
	size_t              size;
	size_t              length;
	char*               value;
	size_t              references;
};

struct
crs_Array {
	struct crs_GCHeader header;
	size_t              size;
	size_t              length;
	struct crs_Object*  value;
	size_t              references;
};

typedef struct crs_String crs_String;
typedef struct crs_Array  crs_Array;

extern int
crsO_compare(crs_Object* a, crs_Object* b);

/*
 * TODO
 *
 * crsO_clone and crsO_deepClone are to be removed once the gc is implemented.
 * these functions not only edit the soon-to-be-obsolete reference counter, but
 * also "deep cloning" is something to implement way later, possibly in
 * metatables or whatever i decide to call those (probably gonna stick with
 * metatables).
 */

extern int
crsO_clone(crs_Object* to, crs_Object* from);

extern int
crsO_deepClone(crs_Object* to, crs_Object* from);

/*
 * TODO
 *
 * crsO_free is to be removed once the gc is implemented. freeing objects will
 * be handled automatically by the gc, so we just need to set the object to nil
 * or remove the reference to it.
 */

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

#define obj_isnumber(o)      ((o)->type & 1)
#define obj_iscollectable(o) ((o)->type & 2)

#define obj_toheader(o) ((crs_GCHeader*)o)
#define obj_tostring(h) ((crs_String*)h)
#define obj_toarray(h)  ((crs_Array*)h)
#define obj_tothread(h) ((crs_Thread*)h)

/* object */
#define obj_seto(a, b) {(a)->type = (b)->type; (a)->value = (b)->value;}

/* nil */
#define obj_setn(o) ((o)->type = CRS_TYPE_NIL)

/* boolean */
#define obj_getb(o)    ((o)->value.b)
#define obj_setb(o, v) {(o)->type = CRS_TYPE_BOOLEAN; (o)->value.b = (v);}

/* integer */
#define obj_geti(o)    ((o)->value.i)
#define obj_seti(o, v) {(o)->type = CRS_TYPE_INTEGER; (o)->value.i = (v);}

/* float */
#define obj_getf(o)    ((o)->value.f)
#define obj_setf(o, v) {(o)->type = CRS_TYPE_FLOAT; (o)->value.f = (v);}

/* cfunction */
#define obj_getc(o)    ((o)->value.c)
#define obj_setc(o, v) {(o)->type = CRS_TYPE_CFUNCTION; (o)->value.c = (v);}

/* gc header */
#define obj_geth(o)    ((o)->value.gc)
#define obj_seth(o, v) {(o)->type = (v)->type; (o)->value.gc = (v);}

/* gc object */
#define obj_gets(o)     obj_tostring(obj_geth(o))
#define obj_geta(o)     obj_toarray(obj_geth(o))
#define obj_setgc(o, v) obj_seth((o), obj_toheader(v))

#endif
