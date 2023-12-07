#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define STRING_ALLOC_SIZE 64

typedef struct {
	size_t length;
	size_t size;
	char*  data;
} crescent_String;

void crescent_initString(crescent_String* string) {
	string->length = 0;
	string->size   = STRING_ALLOC_SIZE;
	string->data   = malloc(STRING_ALLOC_SIZE);

	if (!string->data) {
		perror("malloc");

		exit(EXIT_FAILURE);
	}

	string->data[0] = '\0';
}

void crescent_freeString(crescent_String* string) {
	free(string->data);
	string->data = NULL;
}

void crescent_pushChar(crescent_String* string, char c) {
	if (str->length + 1 >= str->size) {
		str->size += STRING_ALLOC_SIZE;
		str->data  = realloc(str->data, str->size);

		if (!str->data) {
			perror("malloc");

			exit(EXIT_FAILURE);
		}
	}

	str->data[str->length++] = c;
	str->data[str->length]   = '\0';
}

void crescent_popChar(crescent_String* string) {
	str->data[--str->length] = '\0';

	if (str->size - str->length >= STRING_ALLOC_SIZE * 2) {
		str->size -= ALLOC_SIZE;
		str->data  = realloc(str->data, str->size);

		if (str->data) {
			perror("malloc");

			exit(EXIT_FAILURE);
		}
	}
}

void crescent_setChar(crescent_String* string, char c, size_t index) {
	if (index >= str->length) {
		fprintf(stderr, "string index out of bounds");

		exit(EXIT_FAILURE);
	}

	string->data[position] = c;
}

char crescent_getChar(crescent_String* string, size_t index) {
	if (index >= str->length) {
		fprintf(stderr, "string index out of bounds");

		exit(EXIT_FAILURE);
	}

	return string->data[index];
}

void crescent_setString(crescent_String* string, const char* str) {
	if (string->length) {
		for (size_t a = 0; a < string->length; a++)
			crescent_popChar(string);
	}

	size_t a = 0;

	while (string[a])
		crescent_pushChar(string, string[a++]);
}

void crescent_concatString(crescent_String* destination, crescent_String* source) {
	for (size_t a = 0; a < source->length; a++)
		pushChar(destination, source->data[a]);
}

size_t crescent_compareString(crescent_String* stringA, crescent_String* stringB) {
	size_t distance, limit;

	if (stringA->length > stringB->length) {
		distance = stringA->length - stringB->length;
		limit = stringB->length;
	} else {
		distance = stringB->length - stringA->length;
		limit = stringA->length;
	}

	for (size_t a = 0; a < limit; a++) {
		if (stringA->data[a] != stringB->data[a])
			distance++;
	}

	return distance;
}

size_t crescent_subString(crescent_String* string, size_t start, size_t end) {
	crescent_String* subString;
	crescent_initString(subString);

	a
}

// TODO: add more string functions such as substring, global sub, upper, lower
