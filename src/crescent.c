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
	printf("- function\n- number\n- number\n- nil\n");

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

	crs_call(thread, -1, 2, 4);

	printf("returned: %d\n", crs_getTop(thread));
	for (int a = 1; a <= crs_getTop(thread); a++) {
		printf("- %s\n", crs_name(thread, a));
	}

	crs_setTop(thread, 0);
}

int
mischievous(crs_Thread* thread) {
	crs_call(thread, 2, 0, 0);

	return 0;
}

void
handling(crs_Thread* thread) {
	crs_pushCFunction(thread, &mischievous);
	crs_pushTable(thread);
	int status = crs_pcall(thread, 1, 2, 0);

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
	crs_call(thread, 1, 2, 0);
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
	crs_checkTop(thread, 32);
	printf("note: 1 == 1.0\n");

	printf("predicted:\n");
	printf("1.0          -> \"b\"\n");
	printf("1            -> \"a\"\n");
	printf("2.3          -> \"hello\"\n");
	printf("2.31         -> \"world\"\n");
	printf("\"str\"        -> \"c\"\n");
	printf("&mischievous -> \"d\"\n");
	printf("{}           -> \"e\"\n");
	printf("true         -> \"f\"\n");

	crs_pushFloat(thread, 1);
	crs_pushInteger(thread, 1);
	crs_pushFloat(thread, 2.3);
	crs_pushFloat(thread, 2.31);
	crs_pushString(thread, "str");
	crs_pushCFunction(thread, &mischievous);
	crs_pushTable(thread);
	crs_pushBoolean(thread, 1);

	crs_pushString(thread, "b");
	crs_pushString(thread, "a");
	crs_pushString(thread, "hello");
	crs_pushString(thread, "world");
	crs_pushString(thread, "c");
	crs_pushString(thread, "d");
	crs_pushString(thread, "e");
	crs_pushString(thread, "f");

	crs_set(thread, 1, 2, 10);
	crs_set(thread, 1, 3, 11);
	crs_set(thread, 1, 4, 12);
	crs_set(thread, 1, 5, 13);
	crs_set(thread, 1, 6, 14);
	crs_set(thread, 1, 7, 15);
	crs_set(thread, 1, 8, 16);
	crs_set(thread, 1, 9, 17);

	crs_pop(thread, 8);

	printf("\nactual:\n");

	crs_get(thread, 1, 2);
	crs_get(thread, 1, 3);
	crs_get(thread, 1, 4);
	crs_get(thread, 1, 5);
	crs_get(thread, 1, 6);
	crs_get(thread, 1, 7);
	crs_get(thread, 1, 8);
	crs_get(thread, 1, 9);

	printf("1.0          -> \"%s\"\n", crs_toString(thread, 11));
	printf("1            -> \"%s\"\n", crs_toString(thread, 10));
	printf("2.3          -> \"%s\"\n", crs_toString(thread, 12));
	printf("2.31         -> \"%s\"\n", crs_toString(thread, 13));
	printf("\"str\"        -> \"%s\"\n", crs_toString(thread, 14));
	printf("&mischevious -> \"%s\"\n", crs_toString(thread, 15));
	printf("{}           -> \"%s\"\n", crs_toString(thread, 16));
	printf("true         -> \"%s\"\n", crs_toString(thread, 17));

	printf("\nupdate 1 to true:\n");
	crs_pop(thread, 8);
	crs_pushBoolean(thread, 1);
	crs_set(thread, 1, 3, -1);
	crs_pop(thread, 1);

	crs_get(thread, 1, 2);
	crs_get(thread, 1, 3);
	crs_get(thread, 1, 4);
	crs_get(thread, 1, 5);
	crs_get(thread, 1, 6);
	crs_get(thread, 1, 7);
	crs_get(thread, 1, 8);
	crs_get(thread, 1, 9);

	printf("1.0          -> \"%s\"\n", crs_toString(thread, 11));
	printf("1            -> \"%s\"\n", crs_toString(thread, 10));
	printf("2.3          -> \"%s\"\n", crs_toString(thread, 12));
	printf("2.31         -> \"%s\"\n", crs_toString(thread, 13));
	printf("\"str\"        -> \"%s\"\n", crs_toString(thread, 14));
	printf("&mischevious -> \"%s\"\n", crs_toString(thread, 15));
	printf("{}           -> \"%s\"\n", crs_toString(thread, 16));
	printf("true         -> \"%s\"\n", crs_toString(thread, 17));

	crs_pop(thread, 8);

	printf("\nset all to nil:\n");

	crs_pushNil(thread);
	crs_set(thread, 1, 2, -1);
	crs_set(thread, 1, 3, -1);
	crs_set(thread, 1, 4, -1);
	crs_set(thread, 1, 5, -1);
	crs_set(thread, 1, 6, -1);
	crs_set(thread, 1, 7, -1);
	crs_set(thread, 1, 8, -1);
	crs_set(thread, 1, 9, -1);

	crs_get(thread, 1, 2);
	crs_get(thread, 1, 3);
	crs_get(thread, 1, 4);
	crs_get(thread, 1, 5);
	crs_get(thread, 1, 6);
	crs_get(thread, 1, 7);
	crs_get(thread, 1, 8);
	crs_get(thread, 1, 9);

	printf("1.0          -> \"%s\"\n", crs_toString(thread, 11));
	printf("1            -> \"%s\"\n", crs_toString(thread, 10));
	printf("2.3          -> \"%s\"\n", crs_toString(thread, 12));
	printf("2.31         -> \"%s\"\n", crs_toString(thread, 13));
	printf("\"str\"        -> \"%s\"\n", crs_toString(thread, 14));
	printf("&mischevious -> \"%s\"\n", crs_toString(thread, 15));
	printf("{}           -> \"%s\"\n", crs_toString(thread, 16));
	printf("true         -> \"%s\"\n", crs_toString(thread, 17));

	crs_checkTop(thread, 16);
	crs_setTop(thread, 0);
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
	crs_pushString(thread, "let's delve deeper into rainbow philosophy -9uj4itu3809p4tjwerift9psdfuisdfsdfsdfsdfsedfhousdghoiusrgohjiudsfghuoidfghuoidfghuodfghuodfghuojidfg");
	crs_pushString(thread, "far beyong that of cloudsdale's mythology -u9iit9ui08j4r5tu809io4er38u09ij98ouiyw34ert4t5rwet4w5r9o8uijetw45r9o8u0ite543r9u8o0jteg980uoet");
	crs_pushString(thread, "it's easy to misjudge that floating city 80io4tujoiedtgjlidfegsjhiklodgfjhiuoihoujuihojhiuhiujiuhjhuijhuijhuijdwfiuhjerfwihuferhuiferwhuiefriuhjerf");
	crs_pushString(thread, "with its luring decor and social psychology jpo0i4w3rjoiwefiojwefdiowefoijwfeojioijwfeoijwfeiojwfeoijwerfoijwefoijfweiojwfeoijfweoijoijwfeiojwfeiojwfe");
	setHash(thread, 2, "im running out of ideas here sdfoisdfijoojisfdjoifewiojf234wiojfewoijwefoijfweoijwfeoijwfeoijwofije", -4);
	setHash(thread, 2, "john egbert rose lalonde ascend descend rise up abscond 0oij23trjiowdfjiowfdsiojwfedijowfedoijwfeoijwfoiejoiwjfeiojwfeoijwfeoijwefoijwfe", -3);
	setHash(thread, 2, "jade harley dave strider they all play sburb and end the world jpierpijowfdjiowfedijwfeoijwefoijwefoijwoefijoijwefoijwfeoijwfe", -2);
	setHash(thread, 2, "harlequin nanna bro mom dad youth roll grandpa pesterchum hella jeff and sweet bro joip2efojifewroijfwediowfeoijwefoijoijwfeoijwefoijwfe", -1);
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
format(crs_Thread* thread) {
	printf("expected output: %c %s %d %p " CRS_INTEGER_FMT " " CRS_FLOAT_FMT " %%\n", 'a', "hello world", 80085, (void*)thread, (crs_Integer)20, -19.2);

	crs_format(thread, "%c %s %d %p %I %F %% %\0s %s", 'a', "hello world", 80085, thread, (crs_Integer)20, -19.2, "if you're seeing this, crsF_vformat is reading past the string when % is followed by the null terminator. have fun!\n");
	printf("actual output:   %s\n", crs_toString(thread, -1));
	printf("\n");
	crs_setTop(thread, 0);
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
		case 'f':
			format(thread);

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
		printf("j: tables\n");
		printf("g: garbage\n");
		printf("i: interning\n");
		printf("f: format\n");
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
