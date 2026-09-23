/*
 * https://github.com/mochji/crescent
 * core/methods.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_CORE_METHODS_H
#define CRS_CORE_METHODS_H

#include <stddef.h>

#include "crescent/conf.h"
#include "limit.h"

#include "core/object.h"

/* order MT */
enum {
    MT_UNM, MT_ADD, MT_SUB, MT_MUL, MT_DIV, MT_POW, MT_MOD,
    MT_BNOT, MT_BAND, MT_BOR, MT_BXOR, MT_SHL, MT_SHR,
    MT_EQ, MT_LT, MT_LE, MT_GT, MT_GE,
    MT_LEN, MT_CONCAT,
    MT_GET, MT_SET, MT_CALL, MT_MT,
    MT_GC,
    MT_COUNT
};

extern char* crsM_names[MT_COUNT];

void crsM_init(crs_Thread* thread);

/* metatables & metamethods */
crs_Table** crsM_getMTP(crs_Thread* thread, crs_Object* obj);
void        crsM_setMT(crs_Thread* thread, crs_Object* obj, crs_Table* mt);
void        crsM_getMT(crs_Thread* thread, crs_Object* obj, crs_Object* mt);
int         crsM_getMM(crs_Thread* thread, crs_Object* mm, crs_Object* obj,
                                           int op);

/* methods */
crs_Integer crsM_length(crs_Thread* thread, crs_Object* obj);
int  crsM_compare(crs_Thread* thread, crs_Object* l, crs_Object* r, int op);
int  crsM_rawArith(crs_Object* l, crs_Object* r, int op, crs_Object* result);
void crsM_arith(crs_Thread* thread, crs_Object* l, crs_Object* r, int op,
                                    crs_Object* result);
void crsM_get(crs_Thread* thread, crs_Object* obj, crs_Object* key,
                                  crs_Object* value, int raw);
void crsM_set(crs_Thread* thread, crs_Object* obj, crs_Object* key,
                                  crs_Object* value, int raw);
void crsM_call(crs_Thread* thread, int args, int wanted, int raw);
int  crsM_pcall(crs_Thread* thread, int args, int wanted);

#define crsM_rawEqual(l, r) crsM_compare(NULL, l, r, MT_EQ)

#endif
