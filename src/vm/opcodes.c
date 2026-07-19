/*
 * https://github.com/mochji/crescent
 * vm/opcodes.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include "crescent/conf.h"
#include "limit.h"

#include "vm/opcodes.h"

/* order OPCODE */
crs_OpMode crsV_mode[] = {
	iABC,  /* OP_MOV     */
	iABx,  /* OP_GETG    */
	iABx,  /* OP_SETG    */
	iABC,  /* OP_LODN    */
	iABC,  /* OP_LODT    */
	iABC,  /* OP_LODF    */
	iAsBx, /* OP_LODI    */
	iABx,  /* OP_LODC    */
	iABx,  /* OP_LODK    */
	iABC,  /* OP_NEWT    */
	iABC,  /* OP_UNM     */
	iABC,  /* OP_ADD     */
	iABC,  /* OP_SUB     */
	iABC,  /* OP_MUL     */
	iABC,  /* OP_DIV     */
	iABC,  /* OP_POW     */
	iABC,  /* OP_MOD     */
	iABC,  /* OP_NOT     */
	iABC,  /* OP_BNOT    */
	iABC,  /* OP_BAND    */
	iABC,  /* OP_BOR     */
	iABC,  /* OP_BXOR    */
	iABC,  /* OP_SHL     */
	iABC,  /* OP_SHR     */
	iABC,  /* OP_EQ      */
	iABC,  /* OP_LE      */
	iABC,  /* OP_LT      */
	iABC,  /* OP_LENGTH  */
	iABC,  /* OP_CONCAT  */
	iABC,  /* OP_GET     */
	iABC,  /* OP_SET     */
	iABC,  /* OP_CALL    */
	iABC,  /* OP_RETURN  */
	iABC,  /* OP_TEST    */
	isAxx  /* OP_JMP     */
};

/* order OPCODE */
const char* crsV_name[] = {
	"MOV",
	"GETG",
	"SETG",
	"LODN",
	"LODT",
	"LODF",
	"LODI",
	"LODC",
	"LODK",
	"NEWT",
	"UNM",
	"ADD",
	"SUB",
	"MUL",
	"DIV",
	"POW",
	"MOD",
	"NOT",
	"BNOT",
	"BAND",
	"BOR",
	"BXOR",
	"SHL",
	"SHR",
	"EQ",
	"LE",
	"LT",
	"LENGTH",
	"CONCAT",
	"GET",
	"SET",
	"CALL",
	"RETURN",
	"TEST",
	"JMP"
};
