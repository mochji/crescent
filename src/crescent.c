/*
 * https://github.com/mochji/crescent
 * crescent.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdio.h>

#include "api/api.h"

void
stack(crs_Thread* thread) {
	printf("pushing then popping ten\n");
	for (int a = 0; a < 10; a++) {
		crs_pushInteger(thread, a);
	}
	printf("top: %d\n", crs_getTop(thread));

	crs_pop(thread, 10);
	printf("top: %d\n", crs_getTop(thread));

	/* ==== */

	printf("\npushing then removing ten\n");
	for (int a = 0; a < 10; a++) {
		crs_pushInteger(thread, a);
	}
	printf("top: %d\n", crs_getTop(thread));

	for (int a = 0; a < 10; a++) {
		crs_remove(thread, 1);
	}
	printf("top: %d\n", crs_getTop(thread));

	/* ==== */

	printf("\npushing ten then setting zero\n");
	for (int a = 0; a < 10; a++) {
		crs_pushInteger(thread, a);
	}
	printf("top: %d\n", crs_getTop(thread));

	crs_setTop(thread, 0);
	printf("top: %d\n", crs_getTop(thread));

	/* ==== */

	printf("\nsetting ten; should all be nil\n");
	crs_setTop(thread, 10);

	for (int a = 0; a < crs_getTop(thread); a++) {
		printf("%s\n", crs_typeName(crs_type(thread, a)));
	}
	crs_setTop(thread, 0);

	/* ==== */

	printf("\nchecking top to 32\n");
	if (!crs_checkTop(thread, 32)) {
		printf("failed!\n");
	}

	for (int a = 0; a < 32; a++) {
		crs_pushNil(thread);
	}
	printf("%d\n", crs_getTop(thread));

	crs_setTop(thread, 0);
}

void
types(crs_Thread* thread) {
	printf("expected types and values:\n");
	printf("- nil\n");
	printf("- boolean, 1\n");
	printf("- number, -5\n");
	printf("- number, 20.913\n");
	printf("- string, startrix is best\n");

	crs_pushNil(thread);
	crs_pushBoolean(thread, 1);
	crs_pushInteger(thread, -5);
	crs_pushFloat(thread, 20.913);
	crs_pushString(thread, "startrix is best");

	printf("actual types and values:\n");

	for (int a = 1; a <= crs_getTop(thread); a++) {
		int type = crs_type(thread, a);

		printf("- %s", crs_typeName(type));

		switch (type) {
			case CRS_TYPE_BOOLEAN:
				printf(", %d", crs_toBoolean(thread, a)); break;
			case CRS_TYPE_INTEGER:
				printf(", " CRS_INTEGER_FMT, crs_toInteger(thread, a)); break;
			case CRS_TYPE_FLOAT:
				printf(", " CRS_FLOAT_FMT, crs_toFloat(thread, a)); break;
			case CRS_TYPE_STRING:
				printf(", %s", crs_toString(thread, a)); break;
		}

		printf("\n");
	}

	crs_setTop(thread, 0);
}

int
test(crs_Thread* thread) {
	printf("passed args: %d\n", crs_getTop(thread));
	for (int a = 1; a <= crs_getTop(thread); a++) {
		printf("- %s\n", crs_typeName(crs_type(thread, a)));
	}

	crs_pushInteger(thread, 413);
	crs_pushInteger(thread, 612);

	printf("expected returned:\n");
	printf("- function\n- number\n- number\n");

	return 3;
}

void
calling(crs_Thread* thread) {
	crs_pushString(thread, "hello, world");
	crs_pushCFunction(thread, &test);

	printf("expected args: %d\n", crs_getTop(thread));
	for (int a = 1; a <= crs_getTop(thread); a++) {
		printf("- %s\n", crs_typeName(crs_type(thread, a)));
	}

	crs_call(thread, -1, 2);

	printf("returned: %d\n", crs_getTop(thread));
	for (int a = 1; a <= crs_getTop(thread); a++) {
		printf("- %s\n", crs_typeName(crs_type(thread, a)));
	}

	crs_setTop(thread, 0);
}

int
mischievous(crs_Thread* thread) {
	crs_pushString(thread, "error message");
	crs_error(thread, 2);

	return 0;
}

void
handling(crs_Thread* thread) {
	int status;

	crs_pushCFunction(thread, &mischievous);
	crs_pCall(thread, 1, 1, &status);

	printf("top: %d\n", crs_getTop(thread));
	for (int a = 1; a <= crs_getTop(thread); a++) {
		printf("- %s\n", crs_typeName(crs_type(thread, a)));
	}

	if (status != CRS_STATUS_OK) {
		printf("errored! %s\n", crs_toString(thread, 1));
	} else {
		printf("ok?\n");
	}

	crs_setTop(thread, 0);
}

void
panicking(crs_Thread* thread) {
	crs_pushCFunction(thread, &mischievous);
	crs_call(thread, 1, 1);
}

void
garbage(crs_Thread* thread) {
	int counter = 0;

	for (;;) {
		crs_pushString(thread, "sdujrfiouwieruiowu4h5rt3w-9485iu8oi3e4ertui8e984oi5thuj345t0-9i34095tuj90o834uj5t9803uj459i0uj390i8o5ujt4iodjfgioujewi4otjdfg");
		crs_pop(thread, 1);
		printf("%d\n", counter++);
	}
}

void
choose(crs_Thread* thread, char option) {
	printf("========\n");

	switch (option) {
		case 's':
			stack(thread);

			break;
		case 't':
			types(thread);

			break;
		case 'c':
			calling(thread);

			break;
		case 'h':
			handling(thread);

			break;
		case 'p':
			panicking(thread);

			break;
		case 'g':
			garbage(thread);

			break;
		case 'q':
			printf("bye :(\n");

			break;
	}

	printf("========\n");
}

int
main(void) {
	crs_Thread* thread = crs_open();

	printf("hi you wanna test stuff\n");

	char option;

	do {
		printf("s: stack\n");
		printf("t: types\n");
		printf("c: calling\n");
		printf("h: handling\n");
		printf("p: panicking\n");
		printf("g: garbage\n");
		printf("q: quit\n");
		printf("> ");

		do {
			option = getchar();
		} while (option == '\n');

		choose(thread, option);
	} while (option != 'q');

	crs_close(thread);

	return 0;
}
