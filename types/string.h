#ifndef STRING_H
#define STRING_H

#define STRING_ALLOC_SIZE     64

#define STRING_MIN_FREE_CHARS 8
#define STRING_MAX_FREE_CHARS 64

typedef struct {
	size_t length;
	size_t size;
	char*  data;
} crescent_String;

void crescent_initString(crescent_String* str);
void crescent_freeString(crescent_String* str);

void crescent_reallocString(crescent_String* str);

void crescent_pushChar(crescent_String* str, char c);
void crescent_popChar(crescent_String* str, char c);

char crescent_getChar(crescent_String* str, size_t index);
void crescent_setChar(crescent_String* str, char c, size_t index);

void crescent_insertChar(crescent_String* str, char c, size_t index);
void crescent_pullChar(crescent_String* str, size_t index);

void crescent_setString(crescent_String* str, const char* string);

void crescent_concatString(crescent_String* destination, crescent_String* source);
void crescent_compareString(crescent_String* strA, crescent_String* strB);

size_t crescent_subString(crescent_String* str, size_t start, size_t end);

void crescent_lowerString(crescent_String* str);

void crescent_upperString(crescent_String* str);

void crescent_reverse(crescent_String* str);

#endif
