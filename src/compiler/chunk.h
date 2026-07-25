/*
 * https://github.com/mochji/crescent
 * compiler/chunk.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_COMPILER_CHUNK_H
#define CRS_COMPILER_CHUNK_H

#include "crescent/conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/call.h"

/* must fit within a byte and leave extra stack space */
#define MAX_LOCALS 200

typedef struct {
    void**    vec;
    unsigned* size;
    unsigned  count;
    unsigned  max;
    size_t    type;
} Data;

enum {
    UOP_UNM, UOP_LEN, UOP_BNOT, UOP_NOT, UOP_NONE
};

/* order BOP */
enum {
    BOP_ADD, BOP_SUB, BOP_MUL, BOP_DIV, BOP_POW, BOP_MOD,
    BOP_CONCAT,
    BOP_SHL, BOP_SHR, BOP_BAND, BOP_BXOR, BOP_BOR,
    BOP_EQ, BOP_NE, BOP_GT, BOP_GE, BOP_LT, BOP_LE,
    BOP_AND, BOP_OR,
    BOP_NONE
};

enum {
    EXP_NIL,
    EXP_TRUE,
    EXP_FALSE,
    EXP_INT,
    EXP_FLOAT,
    EXP_STRING,
    EXP_LAMBDA,

    EXP_GLOBAL, /* s = name (assignable)       */
    EXP_LOCAL,  /* v = register (assignable)   */
    EXP_TEMP,   /* v = register                */
    EXP_CALL,   /* v = OP_CALL pc              */
    EXP_LIST,   /* v = last evaluated register */
    EXP_INDEX   /* x = index info (assignable) */
};

/* result of an expression */
typedef struct {
    union {
        crs_Integer   i;
        crs_Float     f;
        crs_String*   s;
        unsigned      v;
        crs_Function* l;
        struct {
            crs_byte obj;
            crs_byte key;
        } x;
    }        value;
    crs_byte type;
} Expression;

/* local variable */
typedef struct {
    crs_String* name;
    crs_byte    reg;
} Variable;

/* backpatching for labels and gotos */
typedef struct {
    crs_String* name;
    unsigned    pc;
} Patch;

/* parser state for all functions */
typedef struct {
    struct {
        Variable* vars;
        Patch*    labels;
        Patch*    gotos;
        unsigned  nV;
        unsigned  nL;
        unsigned  nG;
    }    vecs;
    Data vars;
    Data labels;
    Data gotos;
} Parser;

typedef struct {
    unsigned fVar; /* first local */
} Scope;

/* parser state for one function */
typedef struct {
    crs_Thread* thread;
    Parser*     parser;
    Scope*      scope;
    crs_byte    regs;
    unsigned    fVar;   /* first local */
    unsigned    fLabel; /* first label */
    unsigned    fGoto;  /* first goto  */

    crs_Function* func;
    crs_Table*    cTable; /* for reusing constants */
    Data          code;
    Data          consts;
    Data          nested;
} Chunk;

noret
crsI_error(Chunk* chunk, char* format, ...);

crs_Function*
crsI_newChunk(Chunk* chunk, crs_Thread* thread, Parser* parser);

crs_Function*
crsI_finish(Chunk* chunk);

unsigned
crsI_nested(Chunk* chunk, crs_Function* func);

unsigned
crsI_emit(Chunk* chunk, crs_instr i);

unsigned
crsI_newConst(Chunk* chunk, crs_Object* value);

#endif
