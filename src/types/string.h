/*
 * https://github.com/mochji/crescent
 * types/string.h
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * MIT License
 */

#ifndef TYPES_STRING_H
#define TYPES_STRING_H

#include <stddef.h>
#include <stdarg.h>

#include "conf.h"

struct
crescent_String {
	size_t size;
	size_t length;
	char*  data;
	size_t references;
};

typedef struct crescent_String crescent_String;

extern crescent_String*
crescentS_new(size_t length);

extern crescent_String*
crescentS_nullString(void);

extern crescent_String*
crescentS_as(char* str);

extern crescent_String*
crescentS_clone(crescent_String* string);

extern void
crescentS_free(crescent_String* string);

extern int
crescentS_resize(crescent_String* string, size_t newLength);

extern int
crescentS_compare(crescent_String* stringA, crescent_String* stringB);

extern size_t
crescentS_hash(char* str);

extern char*
crescentS_copy(char* str);

extern int
crescentS_hexValue(char c);

extern crescent_Integer
crescentS_toInteger(char* str, int* success);

extern crescent_Float
crescentS_toFloat(char* str, int* success);

#endif
