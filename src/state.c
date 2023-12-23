#include <stdlib.h>
#include <stddef.h>

#include "prefix.h"
#include "conf.h"

#include "object.h"

typedef struct crescent_State {
	struct crescent_Stack {
		size_t           size;
		size_t           pointer;
		size_t           base;
		crescent_Object* data;
	} stack;
	crescent_Table* globals;
};


