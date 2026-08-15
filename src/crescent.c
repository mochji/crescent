/*
 * https://github.com/mochji/crescent
 * crescent.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "crescent/api.h"

/*
 * you're gonna have to manually assemble code into the dump format yourself if
 * you wanna test stuff rn bc there's no compiler :/ (or you could write an
 * assembler, i guess).
 *
 * all that's provided is a basic global "print"
 */

int print(crs_Thread* thread) {
    for (int i = 1; i <= crs_getTop(thread); i++) {
        if (i > 1) {
            printf("\t");
        }

        switch (crs_type(thread, i)) {
            case CRS_TYPE_NIL:
                printf("nil");
                break;
            case CRS_TYPE_BOOLEAN:
                printf(crs_toBoolean(thread, i) ? "true" : "false");
                break;
            case CRS_TYPE_INTEGER:
                printf(CRS_INTEGER_FMT, crs_toInteger(thread, i));
                break;
            case CRS_TYPE_FLOAT:
                printf(CRS_FLOAT_FMT, crs_toFloat(thread, i));
                break;
            case CRS_TYPE_STRING:
                printf("%s", crs_toString(thread, i));
                break;
            case CRS_TYPE_CFUNCTION:
            case CRS_TYPE_TABLE:
            case CRS_TYPE_FUNCTION:
            case CRS_TYPE_THREAD:
                printf("%s", crs_name(thread, i));
                break;
        }
    }

    printf("\n");
    return 0;
}

static int reader(crs_Thread* thread, void* data, char* buffer, int count) {
    (void)thread;
    return fread(buffer, sizeof(char), count, data);
}

static void writer(crs_Thread* thread, void* data, char* buffer, int count) {
    (void)thread;
    fwrite(buffer, sizeof(char), count, data);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "expected one argument\n");
        return 1;
    }

    crs_Thread* thread = crs_open();

    if (thread == NULL) {
        fprintf(stderr, "failed opening thread\n");
        return 1;
    }

    crs_pushString(thread, "print");
    crs_pushCFunction(thread, &print);
    crs_set(thread, CRS_GLOBALS, 1, 2);
    crs_pop(thread, 2);

    FILE* in  = fopen(argv[1], "rb");
    FILE* out = fopen("out.crsc", "wb");
    int   status;

    if ((status = crs_load(thread, &reader, in)) != CRS_STATUS_OK) {
        fprintf(stderr, "error loading dump: %s\n", crs_toString(thread, 1));
        return 1;
    }

    if ((status = crs_pcall(thread, 1, 0, 0)) != CRS_STATUS_OK) {
        fprintf(stderr, "%s\n", crs_toString(thread, 2));
    }

    if ((status = crs_dump(thread, 1, &writer, out)) != CRS_STATUS_OK) {
        fprintf(stderr, "error writing dump: %s\n", crs_toString(thread, 2));
        return 1;
    }

    crs_close(thread);
    return 0;
}
