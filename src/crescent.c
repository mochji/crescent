#include <stdio.h>

#include "api/api.h"

int
main() {
	crs_Thread* thread = crs_open();

	printf("thread address: %p\n", (void*)thread);

	crs_close(thread);

	return 0;
}
