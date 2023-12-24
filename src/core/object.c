#include <stdlib.h>
#include <stddef.h>

#include "../prefix.h"
#include "../conf.h"

struct crescent_String;
struct crescent_Array;

typedef        int              crescent_Boolean;
typedef        CRESCENT_INTEGER crescent_Integer;
typedef        CRESCENT_FLOAT   crescent_Float;
typedef struct crescent_String  crescent_String;
typedef struct crescent_Array   crescent_Array;

typedef enum {
	TYPE_NULL    = 0,
	TYPE_BOOLEAN = 1,
	TYPE_INTEGER = 2,
	TYPE_FLOAT   = 3,
	TYPE_STRING  = 4,
	TYPE_ARRAY   = 5
} crescent_Type;

typedef struct {
	crescent_Type type;
	union crescent_Value {
		crescent_Boolean b;
		crescent_Integer i;
		crescent_Float   f;
		crescent_String* s;
		crescent_Array*  a;
	} value;
} crescent_Object;

struct crescent_String {
	size_t size;
	size_t length;
	char*  data;
};

struct crescent_Array {
	size_t            size;
	size_t            length;
	crescent_Object** data;
};
