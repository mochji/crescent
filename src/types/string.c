/*
 * https://github.com/mochji/crescent
 * types/string.c
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

#include "conf.h"
#include "limit.h"

#include "core/object.h"

#include "types/string.h"

crs_String*
crsS_new(size_t length) {
	crs_String* string = malloc(sizeof(crs_String));

	if (string == NULL) {
		return NULL;
	}

	/* TODO: properly set up header once gc is implemented */
	string->header.type = CRS_TYPE_STRING;

	string->size       = length + CRS_STRING_ALLOCSPACE;
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

crs_String*
crsS_nullString(void) {
	crs_String* string = malloc(sizeof(crs_String));

	if (string == NULL) {
		return NULL;
	}

	/* TODO: properly set up header once gc is implemented */
	string->header.type = CRS_TYPE_STRING;

	string->size       = 0;
	string->length     = 0;
	string->value      = NULL;
	string->references = 1;

	return string;
}

crs_String*
crsS_as(char* str) {
	size_t      length = strlen(str);
	crs_String* string = crsS_new(length);

	if (string == NULL) {
		return NULL;
	}

	for (size_t a = 0; a < length; a++) {
		string->value[a] = str[a];
	}

	string->value[length] = '\0';

	return string;
}

crs_String*
crsS_clone(crs_String* string) {
	crs_String* cloned = malloc(sizeof(crs_String));

	if (cloned == NULL) {
		return NULL;
	}

	/* TODO: properly set up header once gc is implemented */
	cloned->header.type = CRS_TYPE_STRING;

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
crsS_free(crs_String* string) {
	if (string == NULL) {
		return;
	}

	free(string->value);
	free(string);
}

int
crsS_resize(crs_String* string, size_t newLength) {
	if (newLength >= string->size || string->size - newLength > string->size / 2) {
		size_t newSize = newLength + CRS_STRING_ALLOCSPACE;
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
crsS_compare(crs_String* stringA, crs_String* stringB) {
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
crsS_hash(char* str) {
	size_t hash = 5381;

	while (*str) {
		hash = ((hash << 5) + hash) + *(str++);
	}

	return hash;
}

char*
crsS_copy(char* str) {
	char* cloned = malloc(strlen(str) + 1);

	if (cloned == NULL) {
		return NULL;
	}

	return strcpy(cloned, str);
}
