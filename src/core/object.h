/*
 * https://github.com/mochji/crescent
 * core/object.h
 *
 * Copyright (C) 2026 mochji
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
 * (see obj_to* macros after the function prototypes)
 */

struct
crs_GCHeader {
	struct crs_GCHeader* next;
	struct crs_GCHeader* set;
	crs_byte             mark;
	crs_byte             type;
};

struct
crs_Object {
	union {
		int                  b;
		crs_Integer          i;
		crs_Float            f;
		crs_CFunction*       c;
		struct crs_GCHeader* gc;
	}        value;
	crs_byte type;
};

typedef struct crs_GCHeader crs_GCHeader;
typedef struct crs_Object   crs_Object;

struct
crs_String {
	crs_GCHeader header;
	crs_Integer  length;
	crs_byte     hashed;
	unsigned     hash;
	char         contents[];
};

struct
crs_TNode {
	struct crs_String* key;
	struct crs_Object  value;
	struct crs_TNode*  next;
	struct crs_TNode*  previous;
};

struct
crs_Table {
	crs_GCHeader       header;
	/* array */
	size_t             size;
	size_t             length;
	struct crs_Object* array;
	/* hashtable */
	crs_byte           nodes; /* log2 size of table  */
	struct crs_TNode*  free;  /* chain of free nodes */
	struct crs_TNode*  table;
};

typedef struct crs_String crs_String;
typedef struct crs_TNode  crs_TNode;
typedef struct crs_Table  crs_Table;

char*
crsO_name(crs_Object* object);

int
crsO_toBoolean(crs_Object* object, int* match);

crs_Integer
crsO_toInteger(crs_Object* object, int* match);

crs_Float
crsO_toFloat(crs_Object* object, int* match);

char*
crsO_toString(crs_Object* object, int* match);

#define obj_isnumber(o)      ((o)->type & 1)
#define obj_iscollectable(o) ((o)->type & 2)

#define obj_toheader(o) ((crs_GCHeader*)o)
#define obj_tostring(h) ((crs_String*)h)
#define obj_totable(h)  ((crs_Table*)h)
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
#define obj_gett(o)     obj_totable(obj_geth(o))
#define obj_getx(o)     obj_tothread(obj_geth(o))
#define obj_setgc(o, v) obj_seth((o), obj_toheader(v))

#endif
