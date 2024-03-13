#include <stdio.h>
#include "api/crescent.h"

int
test(crescent_State* state) {
	char* string = crescent_toString(state, 1);

	printf("%s\n", string);

	return 0;
}

int
main() {
	crescent_State* state = crescent_openState();

	crescent_pushString(state, "this is a string!!!");
	crescent_callC(state, &test, 1);

	crescent_closeState(state);

	return 0;
}
