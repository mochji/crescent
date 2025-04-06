/*
 * https://github.com/mochji/crescent
 * vm/opcodes.c
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#include "conf.h"

#include "vm/opcodes.h"

crescent_OpMode
crescentV_mode[] = {
	iAB,  /* OP_MOV     */

	iA,   /* OP_LODN    */
	iA,   /* OP_LODT    */
	iA,   /* OP_LODF    */
	iA,   /* OP_LODM    */
	iABx, /* OP_LODPI   */
	iABx, /* OP_LODNI   */
	iA,   /* OP_LODS    */
	iA,   /* OP_LODA    */
	iABx, /* OP_LODC    */
	iABx, /* OP_LODCX   */

	iAB,  /* OP_GETG    */
	iAB,  /* OP_SETG    */

	iABC, /* OP_ADD     */
	iABC, /* OP_SUB     */
	iABC, /* OP_MUL     */
	iABC, /* OP_POW     */
	iABC, /* OP_DIV     */
	iABC, /* OP_MOD     */

	iA,   /* OP_INC     */
	iA,   /* OP_DEC     */

	iAB,  /* OP_BNOT    */
	iABC, /* OP_BAND    */
	iABC, /* OP_BNAND   */
	iABC, /* OP_BOR     */
	iABC, /* OP_BNOR    */
	iABC, /* OP_BXOR    */
	iABC, /* OP_BXNOR   */
	iABC, /* OP_BSL     */
	iABC, /* OP_BSR     */

	iAB,  /* OP_EVAL    */
	iABC, /* OP_EQ      */
	iABC, /* OP_NE      */
	iABC, /* OP_GT      */
	iABC, /* OP_GE      */
	iABC, /* OP_LT      */
	iABC, /* OP_LE      */

	iABC, /* OP_LENGTH  */
	iAB,  /* OP_CONCAT  */
	iABC, /* OP_GET     */
	iABC, /* OP_SET     */

	iABC, /* OP_CALL    */
	iABC, /* OP_TCALL   */
	i,    /* OP_RETURN0 */
	iA,   /* OP_RETURN1 */
	iAB,  /* OP_RETURN  */

	iAxx, /* OP_JMPF    */
	iAxx, /* OP_JMPB    */

	iA,   /* OP_JIS     */
	iAB,  /* OP_JE      */
	iAB,  /* OP_JNE     */
	iAB,  /* OP_JG      */
	iAB,  /* OP_JGE     */
	iAB,  /* OP_JL      */
	iAB   /* OP_JLE     */
};

const char*
crescentV_name[] = {
	"OP_MOV",

	"OP_LODN",
	"OP_LODT",
	"OP_LODF",
	"OP_LODM",
	"OP_LODPI",
	"OP_LODNI",
	"OP_LODS",
	"OP_LODA",
	"OP_LODC",
	"OP_LODCX",

	"OP_GETG",
	"OP_SETG",

	"OP_ADD",
	"OP_SUB",
	"OP_MUL",
	"OP_POW",
	"OP_DIV",
	"OP_MOD",

	"OP_INC",
	"OP_DEC",

	"OP_BNOT",
	"OP_BAND",
	"OP_BNAND",
	"OP_BOR",
	"OP_BNOR",
	"OP_BXOR",
	"OP_BXNOR",
	"OP_BSL",
	"OP_BSR",

	"OP_EVAL",
	"OP_EQ",
	"OP_NE",
	"OP_GT",
	"OP_GE",
	"OP_LT",
	"OP_LE",

	"OP_LENGTH",
	"OP_CONCAT",
	"OP_GET",
	"OP_SET",

	"OP_CALL",
	"OP_TCALL",
	"OP_RETURN0",
	"OP_RETURN1",
	"OP_RETURN",

	"OP_JMPF",
	"OP_JMPB",

	"OP_JIS",
	"OP_JE",
	"OP_JNE",
	"OP_JG",
	"OP_JGE",
	"OP_JL",
	"OP_JLE"
};
