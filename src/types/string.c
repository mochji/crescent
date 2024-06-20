/*
 * https://github.com/mochji/crescent
 * types/string.c
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * MIT License
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "conf.h"

#include "types/string.h"

crescent_String*
crescentS_new(size_t length) {
	crescent_String* string = malloc(sizeof(crescent_String));

	if (string == NULL) {
		return NULL;
	}

	string->size       = length + CRESCENT_STRING_ALLOCSPACE;
	string->length     = length;
	string->value      = malloc(string->size);
	string->references = 1;

	if (string->value == NULL) {
		free(string);

		return NULL;
	}

	string->value[0] = '\0';

	return string;
}

crescent_String*
crescentS_nullString(void) {
	crescent_String* string = malloc(sizeof(crescent_String));

	if (string == NULL) {
		return NULL;
	}

	string->size       = 0;
	string->length     = 0;
	string->value      = NULL;
	string->references = 1;

	return string;
}

crescent_String*
crescentS_as(char* str) {
	size_t           length = strlen(str);
	crescent_String* string = crescentS_new(length);

	if (string == NULL) {
		return NULL;
	}

	for (size_t a = 0; a < length; a++) {
		string->value[a] = str[a];
	}

	string->value[length] = '\0';

	return string;
}

crescent_String*
crescentS_clone(crescent_String* string) {
	crescent_String* cloned = malloc(sizeof(crescent_String));

	if (cloned == NULL) {
		return NULL;
	}

	cloned->size       = string->size;
	cloned->length     = string->length;
	cloned->value      = malloc(string->size);
	cloned->references = 1;

	if (cloned->value == NULL) {
		free(cloned);

		return NULL;
	}

	for (size_t a = 0; a <= string->length; a++) {
		cloned->value[a] = string->value[a];
	}

	return cloned;
}

void
crescentS_free(crescent_String* string) {
	if (string == NULL) {
		return;
	}

	free(string->value);
	free(string);
}

int
crescentS_resize(crescent_String* string, size_t newLength) {
	if (newLength >= string->size || string->size - newLength > string->size / 2) {
		size_t newSize = newLength + CRESCENT_STRING_ALLOCSPACE;
		char*  newData = realloc(string->value, newSize);

		if (newData == NULL) {
			return 1;
		}

		string->size = newSize;
		string->value = newData;
	}

	return 0;
}

int
crescentS_compare(crescent_String* stringA, crescent_String* stringB) {
	if (stringA == stringB) {
		return 1;
	}

	if (stringA->length != stringB->length) {
		return 0;
	}

	return strcmp(stringA->value, stringB->value) == 0;
}

/* djb2 */

size_t
crescentS_hash(char* str) {
	size_t hash = 5381;

	while (*str) {
		hash = ((hash << 5) + hash) + *(str++);
	}

	return hash;
}

char*
crescentS_copy(char* str) {
	char* cloned = malloc(strlen(str) + 1);

	if (cloned == NULL) {
		return NULL;
	}

	return strcpy(cloned, str);
}

int
crescentS_hexValue(char c) {
	if (isdigit(c)) {
		return c - '0';
	}

	return (tolower(c) - 'a') + 10;
}

static int
crescentS_bToInteger(char* str, crescent_Integer* result) {
	crescent_Integer value = 0;

	int  success = 1;
	char c;

	while ((c = *(str++))) {
		if (c != '0' && c != '1') {
			value   = 0;
			success = 0;

			break;
		}

		value *= 2;
		value += c - '0';
	}

	if (result != NULL) {
		*result = value;
	}

	return success;
}

static int
crescentS_dToInteger(char* str, crescent_Integer* result) {
	crescent_Integer value = 0;

	int  success = 1;
	char c;

	while ((c = *(str++))) {
		if (!isdigit(c)) {
			value   = 0;
			success = 0;

			break;
		}

		value *= 10;
		value += c - '0';
	}

	if (result != NULL) {
		*result = value;
	}

	return success;
}

static int
crescentS_xToInteger(char* str, crescent_Integer* result) {
	crescent_Integer value = 0;

	int  success = 1;
	char c;

	while ((c = *(str++))) {
		if (!isxdigit(c)) {
			value   = 0;
			success = 0;

			break;
		}

		value *= 16;
		value += crescentS_hexValue(c);
	}

	if (result != NULL) {
		*result = value;
	}

	return success;
}

crescent_Integer
crescentS_toInteger(char* str, int* success) {
	crescent_Integer value;
	int              negative = 0;

	if (str[0] == '-') {
		negative = 1;
		str     += 1;
	} else if (str[0] == '+') {
		str += 1;
	}

	int successful;

	if (str[0] == '0' && tolower(str[1]) == 'b') {
		successful = crescentS_bToInteger(str + 2, &value);
	} else if (str[0] == '0' && tolower(str[1]) == 'x') {
		successful = crescentS_xToInteger(str + 2, &value);
	} else {
		successful = crescentS_dToInteger(str, &value);
	}

	if (negative && successful) {
		value = -value;
	}

	if (success != NULL) {
		*success = successful;
	}

	return value;
}

/* TODO: support for exponents (e12, e-6, E+50, you get it) */

crescent_Float
crescentS_toFloat(char* str, int* success) {
	crescent_Float value    = 0;
	int            negative = 0;
	int            afterDot = 0;
	crescent_Float scale    = 0.1;

	if (str[0] == '-') {
		negative = 1;
		str     += 1;
	} else if (str[0] == '+') {
		str += 1;
	}

	int  successful = 1;
	char c;

	while ((c = *(str++))) {
		if (c == '.' && !afterDot) {
			afterDot = 1;

			continue;
		}

		if (!isdigit(c)) {
			value      = 0;
			negative   = 0;
			successful = 0;

			break;
		}

		if (afterDot) {
			value += (c - '0') * scale;
			scale /= 10;
		} else {
			value *= 10;
			value += c - '0';
		}
	}

	if (negative) {
		value = -value;
	}

	if (success != NULL) {
		*success = successful;
	}

	return value;
}

/*
 * TODO: make dynamic string format function from scratch, add crescent format codes like %I and
 * %F for crescent_Integer and crescent_Float
 */
