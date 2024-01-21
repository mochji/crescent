#include <stdlib.h>
#include <stddef.h>

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

crescent_State*
crescent_openState() {
	crescent_State* state;
	state = malloc(sizeof(crescent_State));

	if (state == NULL) {
		return NULL;
	}

	state->stack.frameCount = 1;
	state->stack.frames     = malloc(sizeof(crescent_Frame));
	state->stack.topFrame   = state->stack.frames;

	if (state->stack.frames == NULL) {
		free(state);

		return NULL;
	}

	state->stack.topFrame->stack.size = CRESCENT_STACK_INITSIZE;
	state->stack.topFrame->stack.top  = 0;
	state->stack.topFrame->stack.data =
		calloc(CRESCENT_STACK_INITSIZE, sizeof(crescent_Object));

	state->stack.topFrame->callType = CALLTYPE_C;

	if (state->stack.topFrame->stack.data == NULL) {
		free(state->stack.frames);
		free(state);

		return NULL;
	}

	return state;
}

void
crescent_closeState(crescent_State* state) {
	crescent_Frame* currentFrame;

	for (size_t a = 0; a < state->stack.frameCount; a++) {
		currentFrame = state->stack.frames[a];

		free(currentFrame->stack.data);
		free(currentFrame);
	}

	free(state);
}
