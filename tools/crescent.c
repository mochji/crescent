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
#include "crescent/aux.h"
#include "crescent/std.h"

/*
 * GUESS WGAT BITCH???? YEAH THATS RIGHT. THERE IS A COMPILER NOW. IT ONLY TOOK
 * TWO AND A HALF YEARS, BUT I FINALLY HAVE A OWRKING LAHNGUAGE!@!!!*(!U@
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
                printf("%s: %p", crs_name(thread, i), crs_toPointer(thread, i));
                break;
        }
    }

    printf("\n");
    return 0;
}

static int reader(crs_Thread* thread, void* data, char* buffer, int* count) {
    (void)thread;
    *count = (int)fread(buffer, sizeof(char), (size_t)*count, data);
    return CRS_OK;
}

static int writer(crs_Thread* thread, void* data, char* buffer, int count) {
    (void)thread;
    fwrite(buffer, sizeof(char), (size_t)count, data);
    return CRS_OK;
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

    crsX_stdLibs(thread, CRS_STD_ALL);

    crs_pushString(thread, "print");
    crs_pushCFunction(thread, &print);
    crs_set(thread, CRS_GLOBALS, 1, 2);
    crs_pop(thread, 2);

    char* inasdasd = argv[1];
    char* outasdas = "out.crsc";

    FILE* in  = fopen(inasdasd, "rb");
    FILE* out = fopen(outasdas, "wb");
    int   status;

    if ((status = crs_load(thread, &reader, in, inasdasd)) != CRS_OK) {
        fprintf(stderr, "%s\n", crs_toString(thread, 1));
        return 1;
    }

    if ((status = crs_pcall(thread, 1, 0, 0)) != CRS_OK) {
        fprintf(stderr, "%s\n", crs_toString(thread, 2));
    }

    if ((status = crs_dump(thread, 1, &writer, out)) != CRS_OK) {
        fprintf(stderr, "%s\n", crs_toString(thread, 2));
        return 1;
    }

    fclose(in);
    fclose(out);

    crs_close(thread);
    return 0;
}
