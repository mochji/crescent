/*
 * https://github.com/mochji/crescent
 * crescent.c
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#include <stdio.h>

#include "api/api.h"

int
main() {
	crs_Thread* thread = crs_open();

	crs_pushNil(thread);
	crs_pushBoolean(thread, 1);
	crs_pushBoolean(thread, 0);
	crs_pushInteger(thread, -9);
	crs_pushFloat(thread, 10.95);
	crs_pushString(thread, "hello, world!");

	printf("==== types ====\n");

	printf("%s\n", crs_typeName(crs_type(thread, 1)));
	printf("%s\n", crs_typeName(crs_type(thread, 2)));
	printf("%s\n", crs_typeName(crs_type(thread, 3)));
	printf("%s\n", crs_typeName(crs_type(thread, 4)));
	printf("%s\n", crs_typeName(crs_type(thread, 5)));
	printf("%s\n", crs_typeName(crs_type(thread, 6)));

	printf("==== values ====\n");

	printf("%d\n", crs_toBoolean(thread, 1));
	printf("%d\n", crs_toBoolean(thread, 2));
	printf("%d\n", crs_toBoolean(thread, 3));
	printf(CRS_INTEGER_FMT "\n", crs_toInteger(thread, 4));
	printf(CRS_FLOAT_FMT "\n", crs_toFloat(thread, 5));
	printf("%s\n", crs_toString(thread, 6));

	crs_close(thread);

	return 0;
}
