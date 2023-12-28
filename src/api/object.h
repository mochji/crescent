#ifndef OBJECT_H
#define OBJECT_H

#include <stddef.h>

#include "../prefix.h"
#include "../conf.h"

typedef CRESCENT_TYPE_BOOLEAN crescent_Boolean;
typedef CRESCENT_TYPE_INTEGER crescent_Integer;
typedef CRESCENT_TYPE_FLOAT   crescent_Float;

enum crescent_Type {
	CRESCENT_TBOOLEAN,
	CRESCENT_TINTEGER,
	CRESCENT_TFLOAT
};

union crescent_Value {
	crescent_Boolean b;
	crescent_Integer i;
	crescent_Float   f;
};

struct crescent_Item {
	enum  crescent_Type  type;
	union crescent_Value value;
};

#endif
