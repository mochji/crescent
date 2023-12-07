#ifndef STRING_H
#define STRING_H

#define STRING_ALLOC_SIZE 64

typedef struct {
	size_t length;
	size_t size;
	char*  data;
} crescent_String;

void crescent_initString(crescent_String* string);
void crescent_freeString(crescent_String* string);

void crescent_pushChar(crescent_String* string, char c);
void crescent_popChar(crescent_String* string);
void crescent_setChar(crescent_String* string, char c, size_t index);

void crescent_setString(crescent_String* string, const char* str);
void crescent_concatString(crescent_String* destination, crescent_String* source);
size_t crescent_compareString(crescent_String* stringA, crescent_String* stringB);

#endif
