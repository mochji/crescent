#include <stdio.h>

#include "api/api.h"

int
test(crs_Thread* thread) {
	crs_error(thread, "error message");

	return 0;
}

int
main() {
	crs_Thread* thread = crs_open();
	int         status;

	crs_pushCFunction(thread, test);
	crs_pCall(thread, 1, 0, &status);

	printf("%s (%d)\n", crs_getError(thread), status);

	crs_close(thread);

	return 0;
}
