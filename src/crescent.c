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
		printf("%s\n", crs_name(thread, a));
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
	printf("- table\n");

	crs_pushNil(thread);
	crs_pushBoolean(thread, 1);
	crs_pushInteger(thread, -5);
	crs_pushFloat(thread, 20.913);
	crs_pushString(thread, "startrix is best");
	crs_pushTable(thread);

	printf("actual types and values:\n");

	for (int a = 1; a <= crs_getTop(thread); a++) {
		int type = crs_type(thread, a);

		printf("- %s", crs_name(thread, a));

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
		printf("- %s\n", crs_name(thread, a));
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
		printf("- %s\n", crs_name(thread, a));
	}

	crs_call(thread, -1, 2);

	printf("returned: %d\n", crs_getTop(thread));
	for (int a = 1; a <= crs_getTop(thread); a++) {
		printf("- %s\n", crs_name(thread, a));
	}

	crs_setTop(thread, 0);
}

int
mischievous(crs_Thread* thread) {
	crs_call(thread, 2, 0);

	return 0;
}

void
handling(crs_Thread* thread) {
	int status;

	crs_pushCFunction(thread, &mischievous);
	crs_pushTable(thread);
	crs_pCall(thread, 1, 2, &status);

	printf("top: %d\n", crs_getTop(thread));
	for (int a = 1; a <= crs_getTop(thread); a++) {
		printf("- %s\n", crs_name(thread, a));
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
	crs_pushTable(thread);
	crs_call(thread, 1, 2);
}

void
garbage(crs_Thread* thread) {
	int counter = 0;

	for (int a = 0; a < 900000; a++) {
		crs_pushString(thread, "sdujrfiouwieruiowu4h5rt3w-9485iu8oi3e4ertui8e984oi5thuj345t0-9i34095tuj90o834uj5t9803uj459i0uj390i8o5ujt4iodjfgioujewi4otjdfg");
		crs_pop(thread, 1);
		printf("%d\n", counter++);
	}
}

void
tables(crs_Thread* thread) {
	crs_pushTable(thread);

	printf("set \"key_a\" to 0\n");
	crs_pushString(thread, "key_a");
	crs_pushInteger(thread, 0);
	crs_set(thread, 1, 2, 3);
	crs_get(thread, 1, 2);
	printf("key_a: " CRS_INTEGER_FMT "\n", crs_toInteger(thread, -1));
	crs_pop(thread, 3);

	printf("set \"key_b\" to 1\n");
	crs_pushString(thread, "key_b");
	crs_pushInteger(thread, 1);
	crs_set(thread, 1, 2, 3);
	crs_get(thread, 1, 2);
	printf("key_b: " CRS_INTEGER_FMT "\n", crs_toInteger(thread, -1));
	crs_pop(thread, 3);

	printf("set \"key_c\" to 2\n");
	crs_pushString(thread, "key_c");
	crs_pushInteger(thread, 2);
	crs_set(thread, 1, 2, 3);
	crs_get(thread, 1, 2);
	printf("key_c: " CRS_INTEGER_FMT "\n", crs_toInteger(thread, -1));
	crs_pop(thread, 3);

	printf("set \"key_a\" to \"hello everypony my name is markiplier\"\n");
	crs_pushString(thread, "key_a");
	crs_pushString(thread, "hello everypony my name is markiplier");
	crs_set(thread, 1, 2, 3);
	crs_get(thread, 1, 2);
	printf("key_a: %s\n", crs_toString(thread, -1));
	crs_pop(thread, 3);

	crs_pushInteger(thread, 0);
	crs_pushInteger(thread, 0);
	crs_set(thread, 1, 2, 3);
	crs_pop(thread, 2);

	crs_pushInteger(thread, 1);
	crs_pushInteger(thread, 1);
	crs_set(thread, 1, 2, 3);
	crs_pop(thread, 2);

	crs_pushInteger(thread, 2);
	crs_pushInteger(thread, 2);
	crs_set(thread, 1, 2, 3);
	crs_pop(thread, 2);

	crs_pushInteger(thread, 3);
	crs_pushString(thread, "non-sequitur");
	crs_set(thread, 1, 2, 3);
	crs_pop(thread, 2);

	for (int a = 1; a <= crs_length(thread, 1); a++) {
		crs_pushInteger(thread, -a);
		crs_get(thread, 1, 2);
		int type = crs_type(thread, -1);

		printf("- %s", crs_name(thread, -1));

		switch (type) {
			case CRS_TYPE_BOOLEAN:
				printf(", %d", crs_toBoolean(thread, -1)); break;
			case CRS_TYPE_INTEGER:
				printf(", " CRS_INTEGER_FMT, crs_toInteger(thread, -1)); break;
			case CRS_TYPE_FLOAT:
				printf(", " CRS_FLOAT_FMT, crs_toFloat(thread, -1)); break;
			case CRS_TYPE_STRING:
				printf(", %s", crs_toString(thread, -1)); break;
		}

		printf("\n");
		crs_pop(thread, 2);
	}

	printf("====\n");

	crs_pushInteger(thread, 1);
	crs_pushNil(thread);
	crs_set(thread, 1, -2, -1);
	crs_pop(thread, 2);

	for (unsigned int a = 0; a < crs_length(thread, 1); a++) {
		crs_pushInteger(thread, (crs_Integer)a);
		crs_get(thread, 1, 2);
		int type = crs_type(thread, -1);

		printf("- %s", crs_name(thread, -1));

		switch (type) {
			case CRS_TYPE_BOOLEAN:
				printf(", %d", crs_toBoolean(thread, -1)); break;
			case CRS_TYPE_INTEGER:
				printf(", " CRS_INTEGER_FMT, crs_toInteger(thread, -1)); break;
			case CRS_TYPE_FLOAT:
				printf(", " CRS_FLOAT_FMT, crs_toFloat(thread, -1)); break;
			case CRS_TYPE_STRING:
				printf(", %s", crs_toString(thread, -1)); break;
		}

		printf("\n");
		crs_pop(thread, 2);
	}

	crs_pushString(thread, "key_b");
	crs_pushNil(thread);
	crs_set(thread, 1, -2, -1);
	crs_get(thread, 1, -2);
	printf("%s\n", crs_toString(thread, -1));

	crs_setTop(thread, 0);
	printf("%d\n", crs_getTop(thread));
}

void
setHash(crs_Thread* thread, int index, char* key, int value) {
	index -= index < 0;
	value -= value < 0;

	crs_pushString(thread, key);
	crs_set(thread, index, -1, value);
	crs_pop(thread, 1);
}

void
great_and_powerful_trixie(crs_Thread* thread) {
	crs_pushTable(thread);
	crs_pushTable(thread);
	setHash(thread, 1, "a", 2);
	setHash(thread, 2, "a", 1);
	crs_pushString(thread, "let's delve deeper into rainbow philosophy");
	crs_pushString(thread, "far beyong that of cloudsdale's mythology");
	crs_pushString(thread, "it's easy to misjudge that floating city");
	crs_pushString(thread, "with its luring decor and social psychology");
	setHash(thread, 2, "im running out of ideas here", -4);
	setHash(thread, 2, "john egbert rose lalonde ascend descend rise up abscond", -3);
	setHash(thread, 2, "jade harley dave strider they all play sburb and end the world", -2);
	setHash(thread, 2, "harlequin nanna bro mom dad youth roll grandpa pesterchum hella jeff and sweet bro", -1);
	crs_pop(thread, 5);
	/* now all those objects are referenced by the first table */

	printf("usage before: %dKiB\n", crs_gc(thread, CRS_GC_USAGE));
	crs_gc(thread, CRS_GC_FULL);
	printf("usage after: %dKiB\n", crs_gc(thread, CRS_GC_USAGE));
	crs_pop(thread, 1);
	printf("popped table\n");
	printf("usage before: %dKiB\n", crs_gc(thread, CRS_GC_USAGE));
	crs_gc(thread, CRS_GC_FULL);
	printf("usage after: %dKiB\n", crs_gc(thread, CRS_GC_USAGE));

	crs_setTop(thread, 0);
}

void
interning(crs_Thread* thread) {
	crs_gc(thread, CRS_GC_FULL);
	printf("usage before: %dKiB\n", crs_gc(thread, CRS_GC_USAGE));
	crs_setGC(thread, CRS_GC_STOP, 1);

	char* ptr = NULL;

	for (int a = 0; a < 5000000; a++) {
		crs_pushString(thread, "it's time you see that this great marquee is the only place you'");
		char* pushed = (char*)crs_toString(thread, -1);
		if (ptr != NULL && ptr != pushed) {
			printf("hahaha, bad!\n");
			break;
		}
		ptr = pushed;
		crs_pop(thread, 1);
	}

	printf("usage after: %dKiB\n", crs_gc(thread, CRS_GC_USAGE));
	crs_setGC(thread, CRS_GC_STOP, 0);
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
		case 'o':
			garbage(thread);

			break;
		case 'j':
			tables(thread);

			break;
		case 'g':
			great_and_powerful_trixie(thread);

			break;
		case 'i':
			interning(thread);

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

	if (thread == NULL) {
		fprintf(stderr, "failed to open state\n");

		return 1;
	}

	printf("hi you wanna test stuff\n");

	char option;

	do {
		printf("s: stack\n");
		printf("t: types\n");
		printf("c: calling\n");
		printf("h: handling\n");
		printf("p: panicking\n");
		printf("o: garbage stress\n");
		printf("q: quit\n");
		printf("j: tables\n");
		printf("g: garbage\n");
		printf("i: interning\n");
		printf("> ");

		do {
			option = getchar();
		} while (option == '\n');

		choose(thread, option);
	} while (option != 'q');

	crs_close(thread);

	return 0;
}
