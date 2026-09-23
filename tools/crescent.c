/*
 * https://github.com/mochji/crescent
 * crescent.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

/* FIXME: this code is for testing purposes and isn't very good */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "crescent/api.h"
#include "crescent/aux.h"
#include "crescent/std.h"

char* progname;
#define REPLPROMPT "> "

#ifdef CRS_USE_READLINE

#include <readline/readline.h>
#include <readline/history.h>

static void REPL_init(void) {
    rl_initialize();
    stifle_history(128);
}

static void REPL_close(void) {
    rl_clear_history();
}

static char* REPL_readline(char* prompt) {
    char* line = readline(prompt);
    add_history(line);
    return line;
}

#else /* CRS_USE_READLINE */

/* TODO: something other than this */

#define REPL_init()  ((void)0)
#define REPL_close() ((void)0)

char REPL_buffer[512];

static char* REPL_readline(char* prompt) {
    printf(prompt);
    fgets(REPL_buffer, 512, stdin);
}

#endif /* !CRS_USE_READLINE */

int    argc;
char** argv;

crs_Thread* thread;
int         status = CRS_OK;
int         done   = 1;

char* in  = NULL;
char* out = NULL;

static void doHelp(void) {
    printf("Usage: %s [options] [--] [script]\n", progname);
    printf("Options:\n");
    printf("  --       : stop handling options\n");
    printf("  -h       : print this message\n");
    printf("  -o [path]: dump [script] to [path] as a binary dump\n");
    printf("  -e [code]: execute [code] before script\n");
    printf("  -r       : enter REPL after script\n");
}

static int doExec(char* code) {
    status = crsX_loadStr(thread, code, "['-e' argument]");
    status = status == CRS_OK ? crs_pcall(thread, 0, 0) : status;

    if (status != CRS_OK) {
        fprintf(stderr, "%s\n", crs_toString(thread, 1));
        return 0;
    }

    return 2;
}

static int doOption(int* options, int i) {
    char* arg = argv[i];

    if (*options) {
        if (!strcmp(arg, "--")) {
            /* stop processing arguments */
            *options = 0;
            return 1;
        } else if (!strcmp(arg, "-o")) {
            /* dump script as bytecode */
            if (i == argc - 1) {
                fprintf(stderr, "expected argument after '-o'\n");
                return 0;
            } else if (out != NULL) {
                fprintf(stderr, "expected one output file\n");
                return 0;
            }

            out = argv[i + 1];
            return 2;
        } else if (!strcmp(arg, "-e")) {
            /* execute argument before script */
            if (i == argc - 1) {
                fprintf(stderr, "expected argument after -e'\n");
                return 0;
            }

            return doExec(argv[i + 1]);
        } else if (!strcmp(arg, "-r")) {
            /* enter REPL after script */
            done = 0;
            return 1;
        } else if (!strcmp(arg, "-h")) {
            /* print help message */
            doHelp();
            return 1;
        } else if (arg[0] == '-') {
            fprintf(stderr, "unrecognized option '%s'\n", arg);
            doHelp();
            done = 1;
            return 0;
        }
    }

    if (in != NULL) {
        fprintf(stderr, "expected one input file\n");
        return 0;
    }

    in = arg;
    return 1;
}

static void doScript(void) {
    int loadOK = crsX_loadFile(thread, in, in);
    crs_copy(thread, 1);
    status = loadOK == CRS_OK ? crs_pcall(thread, 0, 0) : loadOK;

    if (status != CRS_OK) {
        fprintf(stderr, "%s\n", crs_toString(thread, 2));
        crs_pop(thread, 1);
    }

    if (loadOK == CRS_OK && out != NULL) {
        if ((status = crsX_dumpFile(thread, 1, out)) != CRS_OK) {
            fprintf(stderr, "dump error: %s\n", crs_toString(thread, 1));
            crs_pop(thread, 1);
        }
    }

    crs_pop(thread, 1);
}

static void doArgs(void) {
    int options = 1;

    for (int i = 1; i < argc;) {
        int result = doOption(&options, i);

        if (!result) {
            status = CRS_ERROR;
            done   = 1; /* malformed cli arguments; exit immediately */
            return;
        } else {
            i += result;
        }
    }

    if (in != NULL) {
        doScript();
    } else {
        done = 0; /* no script to execute; do REPL */

        if (out != NULL) {
            fprintf(stderr, "expected input file\n");
            status = CRS_ERROR;
        }
    }
}

static void doREPL(void) {
    char* line;
    int   lineOK;
    printf(CRS_NOTICE "\n");
    REPL_init();

    while ((line = REPL_readline(REPLPROMPT)) != NULL) {
        lineOK = crsX_loadStr(thread, line, "[stdin]");

        if (lineOK == CRS_OK) {
            lineOK = crs_pcall(thread, 0, 0);
        }

        if (lineOK != CRS_OK) {
            fprintf(stderr, "%s\n", crs_toString(thread, 1));
            crs_pop(thread, 1);
        }
    }

    REPL_close();
}

static int protected(crs_Thread* unused) {
    crsX_stdLibs(thread, CRS_STD_ALL);
    doArgs();

    if (!done) {
        doREPL();
    }

    (void)unused;
    return 0;
}

int main(int argcM, char* argvM[]) {
    progname = argvM[0];
    thread   = crs_open();
    argc     = argcM;
    argv     = argvM;

    if (thread == NULL) {
        fprintf(stderr, "failed opening thread\n");
        return EXIT_FAILURE;
    }

    crs_pushCFunction(thread, &protected);

    if (crs_pcall(thread, 0, 0) != CRS_OK) {
        fprintf(stderr, "unprotected error: %s\n", crs_toString(thread, 1));
        status = CRS_ERROR;
    }

    crs_close(thread);
    return status == CRS_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
