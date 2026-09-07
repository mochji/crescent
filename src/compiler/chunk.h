/*
 * https://github.com/mochji/crescent
 * compiler/chunk.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_COMPILER_CHUNK_H
#define CRS_COMPILER_CHUNK_H

#include <limits.h>

#include "crescent/conf.h"
#include "limit.h"

#include "core/object.h"
#include "vm/opcodes.h"

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

#define EXP_NIL     1 /* 00000 001                       */
#define EXP_TRUE    9 /* 00001 001                       */
#define EXP_FALSE  17 /* 00010 001                       */
#define EXP_INT    25 /* 00011 001; i = value            */
#define EXP_FLOAT  33 /* 00100 001; f = value            */
#define EXP_STRING  0 /* 00000 000; s = value            */
#define EXP_GLOBAL  4 /* 00000 100; s = name             */
#define EXP_LOCAL   6 /* 00000 110; v = register         */
#define EXP_TEMP    2 /* 00000 010; v = register         */
#define EXP_FUNC    8 /* 00001 000; v = nested index     */
#define EXP_INDEX  12 /* 00001 100; x = index info       */
#define EXP_CALL   24 /* 00011 000; v = instruction pc   */
#define EXP_LIST   16 /* 00010 000; v = # of items       */
#define EXP_VLIST  40 /* 00101 000; v = # of fixed items */
#define EXP_VOID   32 /* 00100 000                       */

#define exp_canfold(e)    ((e)->type & 1)
#define exp_inreg(e)      ((e)->type & 2)
#define exp_assignable(e) ((e)->type & 4)
#define exp_isvar(e)      ((e)->type == EXP_LOCAL || (e)->type == EXP_GLOBAL)
#define exp_multival(e)   ((e)->type == EXP_CALL)
#define exp_haseffect(e)  ((e)->type == EXP_CALL)

/* result of an expression */
typedef struct {
    union {
        crs_Integer i;
        crs_Float   f;
        crs_String* s;
        unsigned    v;
        struct {
            crs_byte obj;
            crs_byte key;
        } x;
    }        value;
    crs_byte type;
} Expression;

/* list of subexpressions on the left hand side of an assignment */
typedef struct SubexpList {
    struct SubexpList* prev;
    Expression         exp;
} SubexpList;

/* local variable */
typedef struct {
    crs_String* name;
    crs_byte    reg;
} Variable;

/* label declaration */
typedef struct {
    crs_String* name;
    unsigned    list; /* list of unresolved jumps to this label */
    unsigned    pc;
} Label;

/* parser state for all functions */
typedef struct {
    struct {
        Variable* vars;
        Label*    labels;
        unsigned  sV;
        unsigned  sL;
    }    vecs;
    crs_Stream* stream; /* only used for line information */
    Data        vars;
    Data        labels;
} Parser;

typedef struct Scope {
    struct Scope* previous;
    unsigned      fL;
    crs_byte      nV;
    crs_byte      isLoop;
    crs_byte      inLoop;
} Scope;

/* parser state for one function */
typedef struct {
    crs_Thread* thread;
    Parser*     parser;
    Scope*      scope;
    crs_byte    regs;
    crs_byte    locals;
    unsigned    fV;
    unsigned    lV;
    int         prevLine;
    int*        line;

    crs_Function* func;
    Data          code;
    Data          consts;
    Data          nested;
    Data          lines;
} Chunk;

noret crsI_error(Chunk* chunk, char* format, ...);
void  crsI_init(crs_Thread* thread, Parser* parser, crs_Stream* stream);
void  crsI_free(crs_Thread* thread, Parser* parser);

/* chunk */
crs_Function* crsI_newChunk(Chunk* chunk, crs_Thread* thread, Parser* parser);
void          crsI_finish(Chunk* chunk);
unsigned      crsI_nested(Chunk* parent, Chunk* chunk);

/* scope */
void crsI_enter(Chunk* chunk, Scope* scope, int loop);
void crsI_leave(Chunk* chunk);

/* code */
unsigned crsI_emit(Chunk* chunk, crs_instr i);

#define crsI_iABC(f, o, a, b, c)  crsI_emit(f, instr_newiABC(o, a, b, c))
#define crsI_iABx(f, o, a, b)     crsI_emit(f, instr_newiABx(o, a, b))
#define crsI_iAsBx(f, o, a, b, s) crsI_emit(f, instr_newiAsBx(o, a, b, s))
#define crsI_iAxx(f, o, a)        crsI_emit(f, instr_newiAxx(o, a))
#define crsI_isAxx(f, o, a, s)    crsI_emit(f, instr_newisAxx(o, a, s))
#define crsI_nextPC(f)            ((f)->code.count)

/* backpatch lists */
void crsI_jump(Chunk* chunk, unsigned* list);
void crsI_jumpTo(Chunk* chunk, unsigned target);
void crsI_backpatch(Chunk* chunk, unsigned list, unsigned target);
void crsI_label(Chunk* chunk, crs_String* name);
void crsI_goto(Chunk* chunk, crs_String* name);
void crsI_patchAll(Chunk* chunk);

#define crsI_patchHere(f, l) crsI_backpatch(f, l, (f)->code.count)

#define PATCH_NONE UINT_MAX /* pc can be at most UINT_MAX - 1 */

/* expressions */
void     crsI_freeExp(Chunk* chunk, Expression* exp);
int      crsI_flatten(Chunk* chunk, Expression* exp);
crs_byte crsI_store(Chunk* chunk, Expression* exp, crs_byte reg);
void     crsI_toAny(Chunk* chunk, Expression* exp);
void     crsI_toTop(Chunk* chunk, Expression* exp);
void     crsI_unary(Chunk* chunk, Expression* exp, int uop);
unsigned crsI_infix(Chunk* chunk, Expression* lhs, int bop);
void     crsI_binary(Chunk* chunk, Expression* lhs, Expression* rhs,
                                   int bop, unsigned pc);
void     crsI_getValues(Chunk* chunk, Expression* exp, crs_byte count);
void     crsI_index(Chunk* chunk, Expression* obj, Expression* key);
void     crsI_call(Chunk* chunk, Expression* obj, Expression* args);
void     crsI_table(Chunk* chunk, Expression* exp);
void     crsI_set(Chunk* chunk, Expression* tbl, Expression* key,
                                Expression* value);
void     crsI_test(Chunk* chunk, Expression* exp, int test);

#define crsI_getAll(f, e) crsI_getValues(f, e, MAX_REGS)

/* variables */
void crsI_var(Chunk* chunk, crs_String* name, Expression* exp);
void crsI_local(Chunk* chunk, crs_String* name);

/* statements */
void crsI_return(Chunk* chunk, Expression* exp);
void crsI_assign(Chunk* chunk, Expression* var, Expression* exp);
void crsI_multiAssign(Chunk* chunk, SubexpList* vars, Expression* values,
                                    unsigned nExps);

#endif
