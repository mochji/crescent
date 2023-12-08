#include <stddef.h>

#include "string.h"
#include "../state/stack.h"
#include "../state/state.h"

typedef crescent_Integer ssize_t;
typedef crescent_Float   double;
typedef crescent_Boolean unsigned int;
// crescent_String is already defined

typedef union {
	crescent_Integer i;
	crescent_Float   f;
	crescent_Boolean b;
	crescent_String* s;
} crescent_Value;

typedef enum {
	CRESCENT_INTEGER,
	CRESCENT_FLOAT,
	CRESCENT_BOOLEAN,
	CRESCENT_STRING
} crescent_Type;

typedef struct {
	crescent_Value value;
	crescent_Type  type;
} crescent_Object;
