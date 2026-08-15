/*
 * https://github.com/mochji/crescent
 * vm/opcodes.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_VM_OPCODES_H
#define CRS_VM_OPCODES_H

#include "crescent/conf.h"
#include "limit.h"

/*
 * Crescent VM instruction formats
 *
 * An instruction is 32 bits wide.
 *
 *       | DDDDDDDD | CCCCCCCC | BBBBBBBB | AAAAAAAA |
 * iABC  | Op. C    | Op. B    | Op. A    | Opcode   |
 * iABx  | Op. B               | Op. A    | Opcode   |
 * iAsBx | Op. B (signed)      | Op. A    | Opcode   |
 * iAxx  | Op. A                          | Opcode   |
 * isAxx | Op. A (signed)                 | Opcode   |
 *
 * - i: OpCode
 * - A: Operand A
 * - B: Operand B
 * - C: Operand C
 * - x: Extended (+8 bits)
 * - s: Signed
 *
 * The sign bit of signed operands is stored at the highest bit of the
 * instruction. Negative values are stored simply, without two's complement.
 */

#define MAX_Bx   bit_1mask(0, 16)
#define MAX_sBx  bit_1mask(0, 15)
#define MAX_Axx  bit_1mask(0, 24)
#define MAX_sAxx bit_1mask(0, 23)

#define instr_get(i, o, l) ((crs_u32)(bit_get((i), bit_1mask(o, l)) >> (o)))
#define instr_set(x, o, l) (bit_get((crs_instr)(x), bit_1mask(0, l)) << (o))
#define instr_signed(x, s) (((crs_s32)(s) * -2 + 1) * ((crs_s32)(x)))

#define instr_opcode(i) instr_get(i, 0, 8)
#define instr_s(i)      instr_get(i, 31, 1)
#define instr_A(i)      instr_get(i, 8, 8)
#define instr_B(i)      instr_get(i, 16, 8)
#define instr_C(i)      instr_get(i, 24, 8)
#define instr_Bx(i)     instr_get(i, 16, 16)
#define instr_sBx(i)    instr_signed(instr_get(i, 16, 15), instr_s(i))
#define instr_Axx(i)    instr_get(i, 8, 24)
#define instr_sAxx(i)   instr_signed(instr_get(i, 8, 23), instr_s(i))

#define instr_setA(x)       instr_set(x, 8, 8)
#define instr_setB(x)       instr_set(x, 16, 8)
#define instr_setC(x)       instr_set(x, 24, 8)
#define instr_setBx(x)      instr_set(x, 16, 16)
#define instr_setsBx(x, s)  (instr_set(x, 16, 15) | instr_set(s, 31, 1))
#define instr_setAxx(x)     instr_set(x, 8, 24)
#define instr_setsAxx(x, s) (instr_set(x, 8, 23) | instr_set(s, 31, 1))

#define instr_newiABC(o, a, b, c) \
    ((o) | instr_setA(a) | instr_setB(b) | instr_setC(c))
#define instr_newiABx(o, a, b)     ((o) | instr_setA(a) | instr_setBx(b))
#define instr_newiAsBx(o, a, b, s) ((o) | instr_setA(a) | instr_setsBx(b, s))
#define instr_newiAxx(o, a)        ((o) | instr_setAxx(a))
#define instr_newisAxx(o, a, s)    ((o) | instr_setsAxx(a, s))

typedef enum {
    iABC,
    iABx,
    iAsBx,
    iAxx,
    isAxx
} crs_OpMode;

/*
 * G = globals
 * R = registers
 * K = constants
 * F = nested functions
 */

/* order OPCODE */
typedef enum {
/*  enum            operation                         operands */
    OP_MOV,      /* R[A] = R[B]                       AB       */
    OP_GETG,     /* R[A] = G[K[Bx]]                   ABx      */
    OP_SETG,     /* G[K[Bx]] = R[A]                   ABx      */

    OP_LODN,     /* R[A] = nil                        A        */
    OP_LODT,     /* R[A] = true                       A        */
    OP_LODF,     /* R[A] = false                      A        */
    OP_LODI,     /* R[A] = sBx                        AsBx     */
    OP_LODC,     /* R[A] = K[Bx]                      ABx      */
    OP_LODK,     /* R[A] = F[Bx]                      ABx      */
    OP_NEWT,     /* R[A] = {}                         A        */

    OP_UNM,      /* R[A] = -R[B]                      AB       */
    OP_ADD,      /* R[A] = R[B] + R[C]                ABC      */
    OP_SUB,      /* R[A] = R[B] - R[C]                ABC      */
    OP_MUL,      /* R[A] = R[B] * R[C]                ABC      */
    OP_DIV,      /* R[A] = R[B] / R[C]                ABC      */
    OP_POW,      /* R[A] = R[B] ^ R[C]                ABC      */
    OP_MOD,      /* R[A] = R[B] % R[C]                ABC      */

    OP_NOT,      /* R[A] = !R[B]                      AB       */

    OP_BNOT,     /* R[A] = ~R[B]                      AB       */
    OP_BAND,     /* R[A] = R[B] & R[C]                ABC      */
    OP_BOR,      /* R[A] = R[B] | R[C]                ABC      */
    OP_BXOR,     /* R[A] = R[B] ~ R[C]                ABC      */
    OP_SHL,      /* R[A] = R[B] << R[C]               ABC      */
    OP_SHR,      /* R[A] = R[B] >> R[C]               ABC      */

    OP_EQ,       /* R[A] = R[B] == R[C]               ABC      */
    OP_LT,       /* R[A] = R[B] < R[C]                ABC      */
    OP_LE,       /* R[A] = R[B] <= R[C]               ABC      */

    OP_LENGTH,   /* R[A] = #R[B]                      AB       */
    OP_CONCAT,   /* R[A] = R[B] .. R[C]               ABC      */
    OP_GET,      /* R[A] = R[B][R[C]]                 ABC      */
    OP_SET,      /* R[B][R[C]] = R[A]                 ABC      */

    OP_CALL,     /* R[A](R[A+1 .. A+B]) (#C results)  ABC      */
    OP_RETURN,   /* return R[B-A+1 .. B] (#A results) AB       */

    OP_TEST,     /* if R[A] then PC++ (skip next)     A        */
    OP_JMP       /* PC += sAxx                        sAxx     */
} crs_OpCode;

extern crs_OpMode  crsV_mode[];
extern const char* crsV_name[];

#endif
