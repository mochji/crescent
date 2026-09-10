/*
 * https://github.com/mochji/crescent
 * crsc.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdio.h>
#include <stddef.h>

#include "crescent/conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/state.h"
#include "vm/opcodes.h"

#include "crescent/api.h"

static void dis_const(crs_Object* object) {
    switch (object->type) {
        case CRS_TYPE_INTEGER:
            printf("integer " CRS_INTEGER_FMT, obj_geti(object));
            break;
        case CRS_TYPE_FLOAT:
            printf("float " CRS_FLOAT_FMT, obj_getf(object));
            break;
        case CRS_TYPE_STRING:
            printf("string \"%s\"", obj_gets(object)->contents);
            break;
    }
}

static void dis_instr(unsigned pc, crs_instr i) {
    crs_instr opcode = instr_opcode(i);
    printf("\t[%u]:\t%s\t", pc, crsV_name[opcode]);

    unsigned u0;
    unsigned u1;
    unsigned u2;

    switch (opcode) {
        /* A */
        case OP_LODN:
        case OP_LODT:
        case OP_LODF:
        case OP_NEWT:
            u0 = (unsigned)instr_A(i);
            printf("%u", u0);
            break;

        /* sAxx */
        case OP_JMP:
            printf("%u", instr_sAxx(i));
            break;

        /* AB */
        case OP_MOV:
        case OP_UNM:
        case OP_BNOT:
        case OP_NOT:
        case OP_LENGTH:
        case OP_RETURN:
        case OP_TEST:
            u0 = (unsigned)instr_A(i);
            u1 = (unsigned)instr_B(i);
            printf("%u %u", u0, u1);
            break;

        /* ABx */
        case OP_GETG:
        case OP_SETG:
        case OP_LODC:
        case OP_LODK:
            u0 = (unsigned)instr_A(i);
            u1 = (unsigned)instr_Bx(i);
            printf("%u %u", u0, u1);
            break;

        /* AsBx */
        case OP_LODI:
            u0 = (unsigned)instr_A(i);
            printf("%u %u", u0, (int)instr_sBx(i));
            break;

        /* ABC */
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_POW:
        case OP_MOD:
        case OP_BAND:
        case OP_BOR:
        case OP_BXOR:
        case OP_SHL:
        case OP_SHR:
        case OP_EQ:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
        case OP_CONCAT:
        case OP_GET:
        case OP_SET:
        case OP_CALL:
            u0 = (unsigned)instr_A(i);
            u1 = (unsigned)instr_B(i);
            u2 = (unsigned)instr_C(i);
            printf("%u %u %u", u0, u1, u2);
            break;
    }
}

static void dis_info(crs_Function* func, crs_instr i, unsigned pc) {
    switch (instr_opcode(i)) {
        case OP_GETG:
        case OP_SETG:
            printf("\t; ");
            dis_const(&func->consts[instr_Bx(i)]);
            break;
        case OP_LODC:
            printf("\t; ");
            dis_const(&func->consts[instr_Bx(i)]);
            break;
        case OP_CALL: {
            unsigned args   = (unsigned)instr_B(i);
            unsigned wanted = (unsigned)instr_C(i);

            if (args == MAX_REGS) {
                printf("\t; all in, ");
            } else {
                printf("\t; %u in, ", args);
            }

            if (wanted == MAX_REGS) {
                printf("all out");
            } else {
                printf("%u out", wanted);
            }

            break;
        }
        case OP_RETURN: {
            unsigned count = (unsigned)instr_B(i);

            if (count == MAX_REGS) {
                printf("\t; all out");
            } else {
                printf("\t; %u out", count);
            }

            break;
        }
        case OP_TEST:
            printf("\t; if %s, to pc %u",
                instr_B(i) ? "true" : "false", pc + 1);
            break;
        case OP_JMP: {
            int jump = (int)instr_sAxx(i);

            if (jump >= 0) {
                printf("\t; to pc %u", pc + (unsigned)jump);
            } else {
                printf("\t; to pc %u", pc - (unsigned)(-jump));
            }

            break;
        }
    }
}

static void dis_var(Debug_Var* var, unsigned i) {
    printf("\t[%u]:\t%s\t%u\t%u -> %u\n", i, var->name->contents,
        var->reg, var->start, var->end);
}

static void dis_func(crs_Function* func) {
    crs_instr*     code   = func->code;
    crs_Object*    consts = func->consts;
    crs_Function** nested = func->nested;

    if (func->flags & FUNC_DEBUG) {
        printf("%s: (%s) ",
            func->flags & FUNC_MAIN ? "main" : "function",
            func->debug.source->contents
        );
    } else {
        printf("%s: ", func->flags & FUNC_MAIN ? "main" : "function");
    }

    printf("%u params, %u registers: {\n",
        func->args, func->top);

    printf("  %u instructions:\n", func->nI);

    for (unsigned pc = 0; pc < func->nI; pc++) {
        dis_instr(pc, code[pc]);
        dis_info(func, code[pc], pc);
        printf("\n");
    }

    printf("\n  %u constants:\n", func->nC);

    for (unsigned i = 0; i < func->nC; i++) {
        printf("\t[%u]:\t", i);
        dis_const(&consts[i]);
        printf("\n");
    }

    if (func->flags & FUNC_DEBUG) {
        Debug_Var* vars = func->debug.vars;

        printf("\n  %u local variables:\n", func->debug.nV);

        for (unsigned i = 0; i < func->debug.nV; i++) {
            dis_var(&vars[i], i);
        }
    }

    printf("}\n");

    for (unsigned i = 0; i < func->nN; i++) {
        printf("\n");
        dis_func(nested[i]);
    }
}

static int reader(crs_Thread* thread, void* data, char* buffer, int* count) {
    (void)thread;
    FILE* f = data;
    *count  = (int)fread(buffer, sizeof(char), (size_t)*count, f);
    return ferror(f) ? CRS_ERROR : CRS_OK;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "expected one argument\n");
        return 1;
    }

    crs_Thread* thread = crs_open();
    FILE*       f;
    int         status;

    if (thread == NULL) {
        fprintf(stderr, "failed opening thread\n");
        return 1;
    }

    if ((f = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "failed opening file\n");
        crs_close(thread);
        return 1;
    }

    if ((status = crs_load(thread, &reader, f, argv[1])) != CRS_OK) {
        fprintf(stderr, "%s\n", crs_toString(thread, 1));
        crs_close(thread);
        return 1;
    }

    printf("%s\n\n", CRS_RELEASE_STR);
    dis_func(obj_getk(thread->stack.top - 1));

    crs_close(thread);
    return status != CRS_OK;
}
