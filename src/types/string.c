/*
 * https://github.com/mochji/crescent
 * types/string.c
 *
 * Copyright (C) 2026 mochji
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
crsS_new(char* contents) {
	size_t      length = strlen(contents);
	crs_String* string = malloc(sizeof(crs_String) + (length + 1) * sizeof(char));

	if (string == NULL) {
		return NULL;
	}

	/* TODO: properly set up header once gc is implemented */
	string->header.type = CRS_TYPE_STRING;

	string->length     = length;
	string->references = 1;
	strcpy(string->contents, contents);

	return string;
}

crs_String*
crsS_clone(crs_String* string) {
	return crsS_new(string->contents);
}

void
crsS_free(crs_String* string) {
	if (string == NULL) {
		return;
	}

	free(string);
}

int
crsS_compare(crs_String* stringA, crs_String* stringB) {
	if (stringA == stringB) {
		return 1;
	}

	if (stringA->length != stringB->length) {
		return 0;
	}

	return strcmp(stringA->contents, stringB->contents) == 0;
}
