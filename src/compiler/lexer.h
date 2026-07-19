/*
 * https://github.com/mochji/crescent
 * compiler/lexer.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_COMPILER_LEXER_H
#define CRS_COMPILER_LEXER_H

#include "conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/buffer.h"

#define KEYWORD_FIRST 0x100

/* order TOKENS */
enum { /* tokens longer than one character */
	TK_IF = KEYWORD_FIRST, TK_ELSE, TK_FOR, TK_WHILE, TK_DO, TK_CONT, TK_BREAK,
	TK_RETURN, TK_LOCAL, TK_FUNC, TK_TRUE, TK_FALSE, TK_NIL, KEYWORD_LAST,

	TK_CONCAT, TK_SHL, TK_SHR, TK_AND, TK_OR, TK_EQ, TK_NE, TK_LE, TK_GE,
	TK_EOF, TK_INT, TK_FLOAT, TK_NAME, TK_STRING
};

typedef struct {
	union {
		crs_Integer i;
		crs_Float   f;
		crs_String* s;
	}   value;
	int type;
} Token;

typedef struct {
	crs_Thread* thread;
	crs_Stream* stream;
	crs_Buffer  buffer;
	crs_Table*  strings;
	Token       token;
	Token       peek;
	int         next;
	struct {
		int   line;
		char* source;
	} info;
} Lexer;

void
crsL_init(crs_Thread* thread, Lexer* lexer, crs_Stream* stream, char* source);

void
crsL_close(Lexer* lexer);

void
crsL_next(Lexer* lexer);

void
crsL_peek(Lexer* lexer);

noret
crsL_error(Lexer* lexer, char* format, ...);

noret
crsL_unexpected(Lexer* lexer);

noret
crsL_expected(Lexer* lexer, int token);

#endif
