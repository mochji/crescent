/*
 * https://github.com/mochji/crescent
 * core/methods.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_CORE_METHODS_H
#define CRS_CORE_METHODS_H

#include "crescent/conf.h"
#include "limit.h"

/* order MT */
enum {
    MT_UNM, MT_ADD, MT_SUB, MT_MUL, MT_DIV, MT_POW, MT_MOD,
    MT_BNOT, MT_BAND, MT_BOR, MT_BXOR, MT_SHL, MT_SHR,
    MT_EQ, MT_LT, MT_LE, MT_GT, MT_GE,
    MT_LEN, MT_CONCAT,
    MT_GET, MT_SET,
    MT_COUNT
};

extern char* crsM_names[MT_COUNT];

int         crsM_equal(crs_Object* l, crs_Object* r);
int         crsM_compare(crs_Thread* thread, crs_Object* l, crs_Object* r,
                                             int op);
crs_Integer crsM_length(crs_Thread* thread, crs_Object* object);
int         crsM_rawArith(crs_Object* o, crs_Object* l, crs_Object* r, int op);
void        crsM_arith(crs_Thread* thread, crs_Object* o, crs_Object* l,
                                           crs_Object* r, int op);
crs_Object* crsM_get(crs_Thread* thread, crs_Object* object, crs_Object* key);
void        crsM_set(crs_Thread* thread, crs_Object* object, crs_Object* key,
                                         crs_Object* value);

void crsM_call(crs_Thread* thread, int args, int wanted);
int  crsM_pcall(crs_Thread* thread, int args, int wanted);

#endif
