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

#include "crescent/conf.h"
#include "limit.h"

/*
 * The collector references objects via crs_GCHeader, which is located at the
 * beginning of every collectable object.
 *
 * (see obj_to* macros after the function prototypes)
 */
typedef struct crs_GCHeader {
    struct crs_GCHeader* next;
    struct crs_GCHeader* set;
    crs_byte             mark;
    crs_byte             type;
} crs_GCHeader;

typedef struct crs_Object {
    union {
        int            b;
        crs_Integer    i;
        crs_Float      f;
        crs_CFunction* c;
        crs_GCHeader*  gc;
    }        value;
    crs_byte type;
} crs_Object;

typedef struct crs_String {
    crs_GCHeader gc;
    crs_Integer  length;
    crs_byte     hashed;
    unsigned     hash;
    char         contents[];
} crs_String;

typedef struct crs_TNode {
    crs_Object        key;
    crs_Object        value;
    struct crs_TNode* next;
    struct crs_TNode* previous;
} crs_TNode;

typedef struct crs_Table {
    crs_GCHeader gc;
    crs_byte     nodes; /* log2 size of table  */
    crs_TNode*   free;  /* chain of free nodes */
    crs_TNode*   table;
} crs_Table;

/* function flags */
#define FUNC_DEBUG (1 << 0) /* has debug info */

typedef struct {
    unsigned pc; /* first pc on line */
    int      line;
} Debug_Line;

typedef struct {
    crs_String* source;
    Debug_Line* lines;
    unsigned    nL; /* # of lines */
} Debug_Info;

typedef struct crs_Function {
    crs_GCHeader gc;
    crs_byte     flags;
    crs_byte     args;
    crs_byte     top;
    unsigned     nI; /* # of instructions */
    unsigned     nC; /* # of constants    */
    unsigned     nN; /* # of nested funcs */
    /*
     * used for gc traversal. same as their 'n*' counterparts, but stores the
     * actual number of valid elements, as an array may be larger than this
     * during loading and compilation.
     */
    unsigned cC;
    unsigned cN;

    crs_instr*            code;
    crs_Object*           consts;
    struct crs_Function** nested;
    Debug_Info            debug; /* only valid if flags & FUNC_DEBUG */
} crs_Function;

extern crs_Object crsO_nilValue;

char* crsO_name(crs_Object* object);
int   crsO_test(crs_Object* object);
int   crsO_toInteger(crs_Object* object, crs_Integer* result, int coerce);
int   crsO_toFloat(crs_Object* object, crs_Float* result, int coerce);
int   crsO_toString(crs_Object* object, char** result);

/* automatic type coercions (if enabled) */
#ifdef CRS_STR2NUM
#define obj_cvtint(o, v)   crsO_toInteger((o), (v), 1)
#define obj_cvtfloat(o, v) crsO_toFloat((o), (v), 1)
#else
#define obj_cvtint(o, v)   crsO_toInteger((o), (v), 0)
#define obj_cvtfloat(o, v) crsO_toFloat((o), (v), 0)
#endif

/* see type enums in crescent/conf.h */
#define obj_isnumber(o)      ((o)->type & 1)
#define obj_iscollectable(o) ((o)->type & 2)

#define obj_toheader(o) ((crs_GCHeader*)o)
#define obj_tostring(h) ((crs_String*)h)
#define obj_totable(h)  ((crs_Table*)h)
#define obj_tofunc(h)   ((crs_Function*)h)
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
#define obj_getk(o)     obj_tofunc(obj_geth(o))
#define obj_getx(o)     obj_tothread(obj_geth(o))
#define obj_setgc(o, v) obj_seth((o), obj_toheader(v))

#endif
