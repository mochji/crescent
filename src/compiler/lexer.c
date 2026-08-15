/*
 * https://github.com/mochji/crescent
 * compiler/lexer.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdarg.h>

#include "crescent/conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/table.h"
#include "core/object.h"
#include "core/state.h"
#include "core/call.h"
#include "core/format.h"

#include "compiler/lexer.h"

#define c_islower(c) ((c) >= 'a' && (c) <= 'z')
#define c_isupper(c) ((c) >= 'A' && (c) <= 'Z')
#define c_isdigit(c) ((c) >= '0' && (c) <= '9')
#define c_isalpha(c) (c_islower(c) || c_isupper(c) || (c) == '_')
#define c_isalnum(c) (c_isalpha(c) || c_isdigit(c))
#define c_isnewl(c)  ((c) == '\n' || (c) == '\r')
#define c_isxdigit(c) \
    (c_isdigit(c) || ((c) >= 'A' && (c) <= 'F') || ((c) >= 'a' && (c) <= 'f'))
#define c_isgraph(c) ((c) >= '!' && (c) <= '~')

/* order TOKENS */
static char* tokens[] = {
    "if", "else", "for", "while", "do", "continue", "break", "return", "local",
    "function", "true", "false", "nil", "hiii. umm something broke",

    "..", "<<", ">>", "&&", "||", "==", "!=", "<=", ">=", "<eof>", "<int>",
    "<float>", "<name>", "<string>"
};

static char* tokenString(crs_Thread* thread, int token) {
    if (token > KEYWORD_FIRST) {
        return tokens[token - KEYWORD_FIRST];
    }

    crs_String* string = crsF_format(thread,
        c_isgraph(token) ? "%c" : "<\\x%x>", token);

    obj_setgc(&thread->error, string); /* anchor */
    return string->contents;
}

#define next(l) ((l)->next = crsR_next((l)->stream))

static int check(Lexer* lexer, int c) {
    if (lexer->next == c) {
        next(lexer);
        return 1;
    }

    return 0;
}

static int checkSet(Lexer* lexer, char* set) {
    while (*set) {
        if (lexer->next == *set++) {
            next(lexer);
            return 1;
        }
    }

    return 0;
}

static int checkSequence(Lexer* lexer, char* str) {
    while (*str) {
        if (lexer->next != *str++) {
            return 0;
        }

        next(lexer);
    }

    return 1;
}

static void newline(Lexer* lexer, int previous) {
    if (lexer->info.line == INT_MAX) {
        crsL_error(lexer, "too many lines");
    }

    lexer->info.line++;

    if (previous == '\n') {
        check(lexer, '\r'); /* \n\r */
    } else {
        check(lexer, '\n'); /* \r\n */
    }
}

static void comment(Lexer* lexer) {
    int start = lexer->info.line;
    int c;

    for (;;) {
        c = lexer->next;
        next(lexer);

        if (checkSequence(lexer, "*/")) {
            break;
        }

        switch (c) {
            case '\n': case '\r':
                newline(lexer, c);
                break;
            case CRS_EOS:
                crsL_error(lexer,
                    "unterminated long comment (started at line %d)", start);
        }
    }
}

/*
 * add all reserved keywords to the string table. reserved keywords are
 * identified by their integer value.
 */
static void string_reserve(Lexer* lexer) {
    crs_Thread* thread  = lexer->thread;
    crs_Table*  strings = lexer->strings;

    for (int i = KEYWORD_FIRST; i < KEYWORD_LAST; i++) {
        crs_String* string = crsS_new(thread, tokens[i - KEYWORD_FIRST]);
        crs_Object* key    = crsC_anchor(thread, obj_toheader(string));
        crs_Object  value;
        obj_seti(&value, i);

        crsT_set(thread, strings, key, &value);
        crsC_unanchor(thread);
    }
}

static crs_String* string_new(Lexer* lexer) {
    crs_Buffer* buffer = &lexer->buffer;
    crs_String* string = crsS_newl(lexer->thread,
        buffer->buffer, buffer->length);

    crsB_clear(buffer);
    return string;
}

/* buffer -> TK_STRING */
static crs_String* string_literal(Lexer* lexer) {
    crs_Thread* thread = lexer->thread;
    crs_String* string = string_new(lexer);
    crs_Object* key    = crsC_anchor(thread, obj_toheader(string));
    crs_TNode*  node   = crsT_find(lexer->strings, key);

    if (node->value.type != CRS_TYPE_NIL) {
        /* reuse string */
        crsC_unanchor(thread);
        return obj_gets(&node->key);
    }

    /* doesn't exist */
    crsT_set(thread, lexer->strings, key, key);
    crsC_unanchor(thread);

    return string;
}

/* buffer -> TK_NAME or reserved keyword */
static int string_token(Lexer* lexer, Token* token) {
    crs_Thread* thread = lexer->thread;
    crs_String* string = string_new(lexer);
    crs_Object* key    = crsC_anchor(thread, obj_toheader(string));
    crs_TNode*  node   = crsT_find(lexer->strings, key);
    int         type   = KEYWORD_LAST;

    switch (node->value.type) {
        case CRS_TYPE_NIL:
            /* string doesn't exist */
            crsT_set(thread, lexer->strings, key, key);
            type           = TK_NAME;
            token->value.s = string;

            break;
        case CRS_TYPE_INTEGER:
            /* reserved keyword */
            type = obj_geti(&node->value);

            break;
        case CRS_TYPE_STRING:
            /* string already exists */
            type           = TK_NAME;
            token->value.s = obj_gets(&node->value);

            break;
    }

    crsC_unanchor(thread);
    return type;
}

static void escape(Lexer* lexer, crs_Buffer* buffer) {
    switch (lexer->next) {
        case 'a':
            crsB_addChar(buffer, '\a'); break;
        case 'b':
            crsB_addChar(buffer, '\b'); break;
        case 'n':
            crsB_addChar(buffer, '\n'); break;
        case 'r':
            crsB_addChar(buffer, '\r'); break;
        case 't':
            crsB_addChar(buffer, '\t'); break;
        case '"':
            crsB_addChar(buffer, '"'); break;
        case '\'':
            crsB_addChar(buffer, '\''); break;
        case '\\':
            crsB_addChar(buffer, '\\'); break;
        case '\n': case '\r': case CRS_EOS:
            crsL_error(lexer, "unterminated escape sequence");
        default:
            crsL_error(lexer, "invalid escape sequence '\\%s'",
                tokenString(lexer->thread, lexer->next));
    }
}

/* 'c' is already consumed */
static int read_number(Lexer* lexer, Token* token, int c) {
    crs_Buffer* buffer   = &lexer->buffer;
    char*       exp      = "eE";
    int         afterExp = 0;

    crsB_addChar(buffer, c);

    if (c == '0') {
        c = lexer->next;

        if (checkSet(lexer, "xX")) {
            crsB_addChar(buffer, c);
            exp = "pP";
        }
    }

    for (;;) {
        c = lexer->next;

        if (afterExp) {
            if (!c_isdigit(c) && c != '.') {
                break;
            }

            next(lexer);
            crsB_addChar(buffer, c);
        } else if (checkSet(lexer, exp)) {
            crsB_addChar(buffer, c);
            c = lexer->next;

            if (checkSet(lexer, "+-")) {
                crsB_addChar(buffer, c);
            }

            afterExp = 1;
        } else if (c_isxdigit(c) || c == '.') {
            next(lexer);
            crsB_addChar(buffer, c);
        } else {
            break;
        }
    }

    if (c_isalpha(c)) {
        crsB_addChar(buffer, c);
    }

    crsB_addChar(buffer, '\0');

    if (crsF_toInteger(buffer->buffer, &token->value.i)) {
        crsB_clear(buffer);
        return TK_INT;
    } else if (crsF_toFloat(buffer->buffer, &token->value.f)) {
        crsB_clear(buffer);
        return TK_FLOAT;
    }

    crsL_error(lexer, "malformed number '%s'", buffer->buffer);
}

/* 'c' is already consumed */
static int read_name(Lexer* lexer, Token* token, int c) {
    crs_Buffer* buffer = &lexer->buffer;
    crsB_addChar(buffer, c);

    while (c_isalnum(lexer->next)) {
        c = lexer->next;
        next(lexer);

        crsB_addChar(buffer, c);
    }

    return string_token(lexer, token);
}

static int read_string(Lexer* lexer, Token* token, int delimiter) {
    crs_Buffer* buffer = &lexer->buffer;
    int         c;

    while ((c = lexer->next) != delimiter) {
        next(lexer);

        switch (c) {
            case '\\':
                escape(lexer, buffer);
                break;
            case '\n': case '\r': case CRS_EOS:
                crsL_error(lexer, "unterminated string");
            default:
                crsB_addChar(buffer, c);
        }
    }

    next(lexer);

    token->value.s = string_literal(lexer);
    return TK_STRING;
}

static int read_longString(Lexer* lexer, Token* token) {
    crs_Buffer* buffer = &lexer->buffer;
    int         start  = lexer->info.line;
    int         c      = lexer->next;

    if (c_isnewl(c)) {
        /* discount first newline */
        next(lexer);
        newline(lexer, c);
    }

    for (;;) {
        c = lexer->next;
        next(lexer);

        if (c == ']' && check(lexer, ']')) {
            break;
        }

        switch (c) {
            case '\n': case '\r':
                newline(lexer, c);
                crsB_addChar(buffer, '\n');

                break;
            case CRS_EOS:
                crsL_error(lexer,
                    "unterminated long string (started at line %d)", start);
            default:
                crsB_addChar(buffer, c);
        }
    }

    token->value.s = string_literal(lexer);
    return TK_STRING;
}

static int nextToken(Lexer* lexer, Token* token) {
    for (;;) {
        int c = lexer->next;
        next(lexer);

        switch (c) {
            case '\n': case '\r':
                newline(lexer, c);
            case ' ': case '\t': case '\v': case '\f':
                break;
            case '!': /* "!" or "!=" */
                if (check(lexer, '=')) {
                    return TK_NE;
                }

                return '!';
            case '&': /* "&" or "&&" */
                if (check(lexer, '&')) {
                    return TK_AND;
                }

                return '&';
            case '|': /* "|" or "||" */
                if (check(lexer, '|')) {
                    return TK_OR;
                }

                return '|';
            case '/': /* "/" or comment */
                if (check(lexer, '/')) {
                    /* single-line comment */
                    do {
                        c = lexer->next;
                        next(lexer);
                    } while (!c_isnewl(c) && c != CRS_EOS);

                    if (c_isnewl(c)) {
                        newline(lexer, c);
                    }

                    break;
                } else if (check(lexer, '*')) {
                    /* multi-line comment */
                    comment(lexer);
                    break;
                }

                return '/';
            case '.': /* ".", "..", or number */
                if (check(lexer, '.')) {
                    return TK_CONCAT;
                } else if (c_isdigit(lexer->next)) {
                    return read_number(lexer, token, c);
                }

                return '.';
            case '"': case '\'':
                return read_string(lexer, token, c);
            case '[': /* "[" or long string */
                if (check(lexer, '[')) {
                    return read_longString(lexer, token);
                }

                return '[';
            case '=': /* "=" or "==" */
                if (check(lexer, '=')) {
                    return TK_EQ;
                }

                return '=';
            case '<': /* "<", "<<", or "<=" */
                if (check(lexer, '<')) {
                    return TK_SHL;
                } else if (check(lexer, '=')) {
                    return TK_LE;
                }

                return '<';
            case '>': /* ">", ">>", or ">=" */
                if (check(lexer, '>')) {
                    return TK_SHR;
                } else if (check(lexer, '=')) {
                    return TK_GE;
                }

                return '>';
            case CRS_EOS:
                return TK_EOF;
            default:
                if (c_isdigit(c)) {
                    return read_number(lexer, token, c);
                } else if (c_isalpha(c)) {
                    return read_name(lexer, token, c);
                }

                return c; /* single-character token */
        }
    }
}

void crsL_init(crs_Thread* thread, Lexer* lexer, crs_Stream* stream,
                                   char* source) {
    lexer->thread      = thread;
    lexer->stream      = stream;
    lexer->strings     = crsT_new(thread);
    lexer->token.type  = TK_EOF;
    lexer->peek.type   = TK_EOF;
    lexer->info.line   = 1;
    lexer->info.source = source;

    crsC_anchor(thread, obj_toheader(lexer->strings));
    crsB_init(thread, &lexer->buffer);
    string_reserve(lexer);
    next(lexer);
}

void crsL_close(Lexer* lexer) {
    crsB_free(&lexer->buffer);
    crsC_unanchor(lexer->thread);
}

void crsL_next(Lexer* lexer) {
    if (lexer->peek.type == TK_EOF) {
        lexer->token.type = nextToken(lexer, &lexer->token);
    } else {
        lexer->token     = lexer->peek;
        lexer->peek.type = TK_EOF;
    }
}

void crsL_peek(Lexer* lexer) {
    if (lexer->peek.type == TK_EOF) {
        lexer->peek.type = nextToken(lexer, &lexer->peek);
    }
}

noret crsL_unexpected(Lexer* lexer) {
    crsL_error(lexer, "unexpected '%s'",
        tokenString(lexer->thread, lexer->token.type));
}

noret crsL_expected(Lexer* lexer, int token) {
    crsL_error(lexer, "expected '%s'; got '%s'",
        tokenString(lexer->thread, token),
        tokenString(lexer->thread, lexer->token.type)
    );
}
