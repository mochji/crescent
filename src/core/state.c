#include <stdlib.h>
#include <stddef.h>

#include "conf.h"

#include "core/object.h"

enum
crescent_CallType {
	CALLTYPE_C,
	CALLTYPE_CRESCENT
};

struct
crescent_Frame {
	union CallInfo {
		struct crescent {
			size_t savedPC;
		} crescent;
		struct c {
		} c;
	} callInfo;
	struct Stack {
		size_t           size;
		size_t           top;
		crescent_Object* data;
	} stack;
	int callType;
};

struct
crescent_Stack {
	size_t                 frameCount;
	struct crescent_Frame* frames;
	struct crescent_Frame* topFrame;
};

struct
crescent_State {
	struct crescent_Stack stack;
};

typedef struct crescent_Frame crescent_Frame;
typedef struct crescent_State crescent_State;
