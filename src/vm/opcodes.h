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
 * S = stack
 * K = constants
 * F = nested functions
 */

/* order OPCODE */
typedef enum {
/*  enum            operation                         operands */
    OP_MOV,      /* S[A] = S[B]                       AB       */
    OP_GETG,     /* S[A] = G[K[Bx]]                   ABx      */
    OP_SETG,     /* G[K[Bx]] = S[A]                   ABx      */

    OP_LODN,     /* S[A] = nil                        A        */
    OP_LODT,     /* S[A] = true                       A        */
    OP_LODF,     /* S[A] = false                      A        */
    OP_LODI,     /* S[A] = sBx                        AsBx     */
    OP_LODC,     /* S[A] = K[Bx]                      ABx      */
    OP_LODK,     /* S[A] = F[Bx]                      ABx      */
    OP_NEWT,     /* S[A] = {}                         A        */

    OP_UNM,      /* S[A] = -S[B]                      AB       */
    OP_ADD,      /* S[A] = S[B] + S[C]                ABC      */
    OP_SUB,      /* S[A] = S[B] - S[C]                ABC      */
    OP_MUL,      /* S[A] = S[B] * S[C]                ABC      */
    OP_DIV,      /* S[A] = S[B] / S[C]                ABC      */
    OP_POW,      /* S[A] = S[B] ^ S[C]                ABC      */
    OP_MOD,      /* S[A] = S[B] % S[C]                ABC      */

    OP_NOT,      /* S[A] = !S[B]                      AB       */

    OP_BNOT,     /* S[A] = ~S[B]                      AB       */
    OP_BAND,     /* S[A] = S[B] & S[C]                ABC      */
    OP_BOR,      /* S[A] = S[B] | S[C]                ABC      */
    OP_BXOR,     /* S[A] = S[B] ~ S[C]                ABC      */
    OP_SHL,      /* S[A] = S[B] << S[C]               ABC      */
    OP_SHR,      /* S[A] = S[B] >> S[C]               ABC      */

    OP_EQ,       /* S[A] = S[B] == S[C]               ABC      */
    OP_LT,       /* S[A] = S[B] < S[C]                ABC      */
    OP_LE,       /* S[A] = S[B] <= S[C]               ABC      */

    OP_LENGTH,   /* S[A] = #S[B]                      AB       */
    OP_CONCAT,   /* S[A] = S[B] .. S[C]               ABC      */
    OP_GET,      /* S[A] = S[B][S[C]]                 ABC      */
    OP_SET,      /* S[B][S[C]] = S[A]                 ABC      */

    OP_CALL,     /* S[A](S[A+1 .. A+B])  (#C results) ABC      */
    OP_RETURN,   /* return S[B-A+1 .. B] (#A results) AB       */

    OP_TEST,     /* if S[A] then PC++ (skip next)     A        */
    OP_JMP       /* PC += sAxx                        sAxx     */
} crs_OpCode;

extern crs_OpMode  crsV_mode[];
extern const char* crsV_name[];

#endif
