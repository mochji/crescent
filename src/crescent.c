/*
 * https://github.com/mochji/crescent
 * crescent.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdio.h>

#include "api/api.h"

int
reallybad(crs_Thread* thread) {
	crs_pushString(thread, "hahaha i'm an evil function ooOOoo");
	crs_error(thread, 1);

	return 0;
}

int
test(crs_Thread* thread) {
	crs_pushString(thread, "hello, world!");

	return 1;
}

int
main(void) {
	crs_Thread* thread = crs_open();

	crs_pushNil(thread);
	crs_pushBoolean(thread, 1);
	crs_pushBoolean(thread, 0);
	crs_pushInteger(thread, -9);
	crs_pushFloat(thread, 10.95);
	crs_pushCFunction(thread, &test);
	crs_call(thread, -1, 1);

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

	for (int a = 0; a < 500; a++) {
		crs_pushString(thread, "p9wuiertiou9ewrothjuiowe4utr980ou3i4h5r-90345ui90348i5t0u934itu90uerfgidfg");
		crs_pop(thread, 1);
	}

	printf("==== error handling ====\n");

	int status;

	crs_setTop(thread, 0);
	crs_pushCFunction(thread, &reallybad);
	crs_pCall(thread, 1, 0, &status);

	if (status != CRS_STATUS_OK) {
		printf("error!\n");

		printf("%s\n", crs_typeName(crs_type(thread, 2)));
		printf("%s\n", crs_toString(thread, 2));
	} else {
		printf("no error?\n");
	}

	crs_close(thread);

	return 0;
}
