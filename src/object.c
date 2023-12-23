#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "conf.h"

#define crescent_internalError(msg) printf("%s\n", msg)

/*
 * ==================================
 * Type declaration
 * ==================================
*/

struct crescent_String;
struct crescent_Array;

typedef        int              crescent_Boolean;
typedef        CRESCENT_INTEGER crescent_Integer;
typedef        CRESCENT_FLOAT   crescent_Float;
typedef struct crescent_String  crescent_String;
typedef struct crescent_Array   crescent_Array;

typedef enum {
	TYPE_BOOLEAN = 0,
	TYPE_INTEGER = 1,
	TYPE_FLOAT   = 2,
	TYPE_STRING  = 3,
	TYPE_ARRAY   = 4
} crescent_Type;

typedef struct {
	int           initialized;
	crescent_Type type;
	union crescent_Value {
		crescent_Boolean b;
		crescent_Integer i;
		crescent_Float   f;
		crescent_String* s;
		crescent_Array*  a;
	} value;
} crescent_Object;

/*
 * ==================================
 * Struct declaration
 * ==================================
*/

struct crescent_String {
	size_t size;
	size_t length;
	char*  data;
};

struct crescent_Array {
	size_t           size;
	size_t           length;
	crescent_Object* data;
};

/*
 * ==================================
 * Object allocation
 * ==================================
*/

crescent_Object*
crescent_newObject(crescent_Type initialType) {
	crescent_Object* object;
	object = malloc(sizeof(crescent_Object));

	if (!object)
		crescent_internalError("malloc failure");

	object->initialized = 0;
	object->type        = initialType;

	switch (initialType) {
		case TYPE_BOOLEAN:
			object->value.b = 0;

			break;
		case TYPE_INTEGER:
			object->value.i = 0;

			break;
		case TYPE_FLOAT:
			object->value.f = 0.0;

			break;
		case TYPE_STRING:
			object->value.s = malloc(sizeof(crescent_String));

			object->value.s->size   = 0;
			object->value.s->length = 0;
			object->value.s->data   = NULL;

			break;
		case TYPE_ARRAY:
			object->value.a = malloc(sizeof(crescent_Array));

			object->value.a->size   = 0;
			object->value.a->length = 0;
			object->value.a->data   = NULL;

			break;
	}

	return object;
}

void
crescent_freeObject(crescent_Object* object) {
	if (!object)
		crescent_internalError("attempt to free a NULL object");

	switch (object->type) {
		case TYPE_STRING:
			if (object->value.s->data)
				free(object->value.s->data);

			break;
		case TYPE_ARRAY:
			if (object->value.a->data)
				free(object->value.a->data);
		default:
			break;

			break;
	}

	free(object);
	object = NULL;
}

crescent_Object*
crescent_cloneObject(crescent_Object* object) {
	crescent_Object* newObject;
	newObject = malloc(sizeof(crescent_Object));

	if (!newObject)
		crescent_internalError("malloc failure");

	newObject->initialized = object->initialized;
	newObject->type        = object->type;

	switch (newObject->type) {
		case TYPE_BOOLEAN:
			newObject->value.b = object->value.b;
			break;
		case TYPE_INTEGER:
			newObject->value.i = object->value.i;
			break;
		case TYPE_FLOAT:
			newObject->value.f = object->value.f;
			break;
		case TYPE_STRING:
			newObject->value.s = malloc(sizeof(crescent_String));

			newObject->value.s->size   = object->value.s->size;
			newObject->value.s->length = object->value.s->length;

			if (newObject->value.s->data) {
				newObject->value.s->data = malloc(newObject->value.s->size);

				for (size_t a = 0; a < newObject->value.s->size; a++)
					newObject->value.s->data[a] = object->value.s->data[a];
			} else {
				newObject->value.s->data = NULL;
			}

			break;
		case TYPE_ARRAY:
			newObject->value.a = malloc(sizeof(crescent_Array));

			newObject->value.a->size   = object->value.a->size;
			newObject->value.a->length = object->value.a->length;

			if (newObject->value.a->data) {
				newObject->value.a->data = malloc(sizeof(crescent_Object) * newObject->value.a->size);

				for (size_t a = 0; a < newObject->value.a->size; a++)
					newObject->value.a->data[a] = object->value.a->data[a];
			} else {
				newObject->value.a->data = NULL;
			}

			break;
	}

	return newObject;
}

/*
 * ==================================
 * Object <-> Stack interaction
 * ==================================
*/
