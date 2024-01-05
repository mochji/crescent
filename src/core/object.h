#ifndef CORE_OBJECT_H
#define CORE_OBJECT_H

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

typedef int64_t crescent_Integer;
typedef double  crescent_Float;

enum crescent_Type {
	CRESCENT_TINTEGER,
	CRESCENT_TFLOAT
};

union crescent_Value {
	crescent_Integer i;
	crescent_Float   f;
};

struct crescent_Object {
	enum  crescent_Type  type;
	union crescent_Value value;
};

typedef enum   crescent_Type   crescent_Type;
typedef union  crescent_Value  crescent_Value;
typedef struct crescent_Object crescent_Object;

#endif
