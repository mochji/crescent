#ifndef CORE_OBJECT_H
#define CORE_OBJECT_H

#include "conf.h"

typedef CRESCENT_TYPE_INTEGER crescent_Integer;
typedef CRESCENT_TYPE_FLOAT   crescent_Float;

enum
crescent_Type {
	TYPE_INTEGER,
	TYPE_FLOAT
};

union
crescent_Value {
	crescent_Integer i;
	crescent_Float   f;
};

struct
crescent_Object {
	enum  crescent_Type  type;
	union crescent_Value value;
};

typedef enum   crescent_Type   crescent_Type;
typedef union  crescent_Value  crescent_Value;
typedef struct crescent_Object crescent_Object;

#endif
