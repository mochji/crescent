/*
 * https://github.com/mochji/crescent
 * vm/opcodes.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include "conf.h"
#include "limit.h"

#include "vm/opcodes.h"

crs_OpMode
crsV_mode[] = {
	iABC,  /* OP_MOV     */
	iABC,  /* OP_LODN    */
	iABC,  /* OP_LODT    */
	iABC,  /* OP_LODF    */
	iAsBx, /* OP_LODI    */
	iABC,  /* OP_LODC    */
	iABC,  /* OP_ADD     */
	iABC,  /* OP_SUB     */
	iABC,  /* OP_MUL     */
	iABC,  /* OP_DIV     */
	iABC,  /* OP_POW     */
	iABC,  /* OP_MOD     */
	iABC,  /* OP_NOT     */
	iABC,  /* OP_AND     */
	iABC,  /* OP_OR      */
	iABC,  /* OP_BNOT    */
	iABC,  /* OP_BAND    */
	iABC,  /* OP_BOR     */
	iABC,  /* OP_BXOR    */
	iABC,  /* OP_EQ      */
	iABC,  /* OP_LE      */
	iABC,  /* OP_LT      */
	iABC,  /* OP_LENGTH  */
	iABC,  /* OP_CONCAT  */
	iABC,  /* OP_GET     */
	iABC,  /* OP_SET     */
	iABC,  /* OP_CALL    */
	iABC,  /* OP_RETURN0 */
	iABC,  /* OP_RETURN1 */
	iABC,  /* OP_RETURN  */
	iABC,  /* OP_TEST    */
	isAxx  /* OP_JMP     */
};

const char*
crsV_name[] = {
	"OP_MOV",
	"OP_LODN",
	"OP_LODT",
	"OP_LODF",
	"OP_LODI",
	"OP_LODC",
	"OP_ADD",
	"OP_SUB",
	"OP_MUL",
	"OP_DIV",
	"OP_POW",
	"OP_MOD",
	"OP_NOT",
	"OP_AND",
	"OP_OR",
	"OP_BNOT",
	"OP_BAND",
	"OP_BOR",
	"OP_BXOR",
	"OP_EQ",
	"OP_LE",
	"OP_LT",
	"OP_LENGTH",
	"OP_CONCAT",
	"OP_GET",
	"OP_SET",
	"OP_CALL",
	"OP_RETURN0",
	"OP_RETURN1",
	"OP_RETURN",
	"OP_TEST",
	"OP_JMP"
};
