/*
 * https://github.com/mochji/crescent
 * compiler/parser.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>

#include "crescent/conf.h"
#include "limit.h"

#include "types/string.h"
#include "core/object.h"
#include "core/state.h"
#include "core/call.h"
#include "core/debug.h"
#include "core/buffer.h"
#include "core/format.h"
#include "compiler/lexer.h"
#include "compiler/chunk.h"

#include "compiler/parser.h"

static int check(Lexer* lexer, int token) {
    crsL_peek(lexer);

    if (lexer->peek.type == token) {
        crsL_next(lexer);
        return 1;
    }

    return 0;
}

static void check_expected(Lexer* lexer, int token) {
    crsL_next(lexer);

    if (lexer->token.type != token) {
        crsL_expected(lexer, token);
    }
}

static crs_String* get_name(Lexer* lexer) {
    check_expected(lexer, TK_NAME);
    return lexer->token.value.s;
}

/*
 * Operator precedence (descending)
 *
 * precedence | operators       | associativity
 * 12         | ^               | right
 * 11         | - # ~ ! (unary) | right
 * 10         | * / %           | left
 * 9          | + -             | left
 * 8          | ..              | left
 * 7          | << >>           | left
 * 6          | &               | left
 * 5          | ~               | left
 * 4          | |               | left
 * 3          | == != > >= < <= | left
 * 2          | &&              | left
 * 1          | ||              | left
 */

/* order BOP */
struct {
    int left;
    int right;
} precedence[] = {
    {9, 9}, {9, 9}, {10, 10}, {10, 10}, {12, 13}, {10, 10},
    {8, 8},
    {7, 7}, {7, 7}, {6, 6}, {4, 4}, {4, 4},
    {3, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3},
    {2, 2}, {1, 1}
};

#define PRECEDENCE_UNARY 11

static int try_uop(Lexer* lexer) {
    crsL_peek(lexer);
    int op = UOP_NONE;

    switch (lexer->peek.type) {
        case '-':
            op = UOP_UNM; break;
        case '#':
            op = UOP_LEN; break;
        case '~':
            op = UOP_BNOT; break;
        case '!':
            op = UOP_NOT; break;
    }

    if (op != UOP_NONE) {
        crsL_next(lexer);
    }

    return op;
}

static int try_bop(Lexer* lexer) {
    crsL_peek(lexer);
    int op = BOP_NONE;

    switch (lexer->peek.type) {
        case '+':
            op = BOP_ADD; break;
        case '-':
            op = BOP_SUB; break;
        case '*':
            op = BOP_MUL; break;
        case '/':
            op = BOP_DIV; break;
        case '^':
            op = BOP_POW; break;
        case '%':
            op = BOP_MOD; break;
        case TK_CONCAT:
            op = BOP_CONCAT; break;
        case TK_SHL:
            op = BOP_SHL; break;
        case TK_SHR:
            op = BOP_SHR; break;
        case '&':
            op = BOP_BAND; break;
        case '~':
            op = BOP_BXOR; break;
        case '|':
            op = BOP_BOR; break;
        case TK_EQ:
            op = BOP_EQ; break;
        case TK_NE:
            op = BOP_NE; break;
        case '>':
            op = BOP_GT; break;
        case TK_GE:
            op = BOP_GE; break;
        case '<':
            op = BOP_LT; break;
        case TK_LE:
            op = BOP_LE; break;
        case TK_AND:
            op = BOP_AND; break;
        case TK_OR:
            op = BOP_OR; break;
    }

    if (op != BOP_NONE) {
        crsL_next(lexer);
    }

    return op;
}

/* forward declarations */
static void     func_body(Lexer* lexer, Expression* exp);
static int      expr(Lexer* lexer, Expression* exp, int power);
static unsigned expr_list(Lexer* lexer, Expression* exp, unsigned wanted);
static void     body(Lexer* lexer);

/*
 * ===========================
 *  values
 * ===========================
 */

/* <table_val> ::= ((<name> | "[" <expr> "]") "=")? <expr> */
static void table_val(Lexer* lexer, Expression* tbl, crs_Integer* len) {
    Chunk*      chunk = lexer->chunk;
    crs_String* name  = NULL;
    Expression  key, value;

    crsL_peek(lexer);

    switch (lexer->peek.type) {
        case '}': /* trailing "," */
            return;
        case '[': /* "[" <expr> "]" "=" <expr> */
            crsL_next(lexer);
            expr(lexer, &key, 0);
            check_expected(lexer, ']');
            crsI_flatten(chunk, &key);

            check_expected(lexer, '=');
            expr(lexer, &value, 0);

            break;
        case TK_NAME: /* <name> "=" <expr> */
            name = lexer->peek.value.s;
            expr(lexer, &key, 0);

            /* actually '<name> "=" <expr>'? */
            if (exp_isvar(&key) && check(lexer, '=')) {
                key.type    = EXP_STRING;
                key.value.s = name;
                expr(lexer, &value, 0);

                break;
            }

            value = key; /* is actually the value, not the key */
            /* fallthrough */
        default: /* <expr> */
            if (name == NULL) {
                expr(lexer, &value, 0);
            }

            key.type    = EXP_INT;
            key.value.i = (*len)++;
    }

    crsI_set(chunk, tbl, &key, &value); /* 'key' and 'value' are freed */
}

/* <table> ::= "{" (<table_val> ("," <table_val>)* ","?)? "}" */
static void table(Lexer* lexer, Expression* tbl) {
    crs_Integer len = 0; /* # of array elements */
    crsI_table(lexer->chunk, tbl);

    if (check(lexer, '}')) {
        return;
    }

    do {
        table_val(lexer, tbl, &len);
    } while (check(lexer, ','));

    check_expected(lexer, '}');
}

/*
 * <value> ::= <nil>
 *           | <true>
 *           | <false>
 *           | <int>
 *           | <float>
 *           | <string>
 *           | <name>
 *           | "function" <func_body>
 *           | <table>
 */
static void value(Lexer* lexer, Expression* exp) {
    crsL_next(lexer);
    Token* token = &lexer->token;

    switch (token->type) {
        case TK_NIL:
            exp->type = EXP_NIL;
            break;
        case TK_TRUE:
            exp->type = EXP_TRUE;
            break;
        case TK_FALSE:
            exp->type = EXP_FALSE;
            break;
        case TK_INT:
            exp->type    = EXP_INT;
            exp->value.i = token->value.i;
            break;
        case TK_FLOAT:
            exp->type    = EXP_FLOAT;
            exp->value.f = token->value.f;
            break;
        case TK_STRING:
            exp->type    = EXP_STRING;
            exp->value.s = token->value.s;
            break;
        case TK_NAME:
            crsI_var(lexer->chunk, token->value.s, exp);
            break;
        case TK_FUNC:
            func_body(lexer, exp);
            break;
        case '{':
            table(lexer, exp);
            break;
        default:
            crsL_unexpected(lexer);
    }
}

/* <index> ::= "[" <expr> "]" | "." <name> */
static int index(Lexer* lexer, Expression* obj) {
    Expression key;
    crsL_peek(lexer);

    switch (lexer->peek.type) {
        case '[':
            crsL_next(lexer);
            crsI_flatten(lexer->chunk, obj);
            expr(lexer, &key, 0);
            check_expected(lexer, ']');
            crsI_index(lexer->chunk, obj, &key);

            return 1;
        case '.':
            crsL_next(lexer);
            key.type    = EXP_STRING;
            key.value.s = get_name(lexer);

            crsI_flatten(lexer->chunk, obj);
            crsI_index(lexer->chunk, obj, &key);

            return 1;
    }

    return 0;
}

/* <call> ::= "(" <expr_list>? ")" */
static int call(Lexer* lexer, Expression* obj) {
    if (!check(lexer, '(')) {
        return 0;
    }

    Expression args;
    crsI_toTop(lexer->chunk, obj);

    if (check(lexer, ')')) {
        args.type = EXP_VOID;
    } else {
        expr_list(lexer, &args, 0);
        check_expected(lexer, ')');
    }

    crsI_call(lexer->chunk, obj, &args); /* 'crsI_call' frees list */
    return 1;
}

/*
 * ===========================
 *  expressions
 * ===========================
 */

/*
 * Expression lists
 *
 * - EXP_VOID:  zero expressions
 * - EXP_LIST:  two or more expressions
 * - EXP_VLIST: zero or more expressions, terminated by a multival expression
 * - other:     just one expression
 *
 * Expression lists assume that the last (fixed) value is located at
 * 'chunk->regs - 1'.
 */

/*
 * if 'wanted' == 0, then allow an arbitrary number of expressions (potentially
 * a VLIST, then)
 */
/* <expr_list> ::= <expr> ("," <expr>)* */
static unsigned expr_list(Lexer* lexer, Expression* exp, unsigned wanted) {
    unsigned count = 0;

    do {
        if (count++) {
            /*
             * previous value must be in the top temporary register. function
             * calls followed by another expression thus use only the first
             * return value.
             */
            crsI_toTop(lexer->chunk, exp);
        }

        expr(lexer, exp, 0);
    } while (check(lexer, ','));

    if (exp_multival(exp)) {
        count--;

        if (!wanted) {
            /* previous fixed values, and all values from expression */
            crsI_getAll(lexer->chunk, exp);
        } else {
            crsI_getValues(lexer->chunk, exp,
                (crs_byte)(count >= wanted ? 0 : wanted - count));
        }

        exp->value.v += count;
        count         = exp->value.v;
    } else if (count > 1) {
        /* last value must be in the top temp reg, too */
        crsI_toTop(lexer->chunk, exp);
        exp->type    = EXP_LIST;
        exp->value.v = count;
    } /* single values are returned as is */

    return count;
}

/* <expr_prim> ::= <value> | "(" <expr> ")" */
static void expr_prim(Lexer* lexer, Expression* exp) {
    if (check(lexer, '(')) {
        expr(lexer, exp, 0);
        check_expected(lexer, ')');
    } else {
        value(lexer, exp);
    }
}

/* <expr_sub> ::= <expr_prim> (<index> | <call>)* */
static void expr_sub(Lexer* lexer, Expression* exp) {
    expr_prim(lexer, exp);
    while (index(lexer, exp) || call(lexer, exp));
}

/* <expr> ::= (<expr_sub> | <op_un> <expr>) (<op_bin> <expr>)* */
static int expr(Lexer* lexer, Expression* exp, int power) {
    int uop = try_uop(lexer);
    int bop = BOP_NONE;

    if (uop != UOP_NONE) {
        bop = expr(lexer, exp, PRECEDENCE_UNARY);
        crsI_unary(lexer->chunk, exp, uop);
    } else {
        expr_sub(lexer, exp);
        bop = try_bop(lexer);
    }

    while (bop != BOP_NONE && precedence[bop].left > power) {
        Expression rhs;

        unsigned pc   = crsI_infix(lexer->chunk, exp, bop);
        int      next = expr(lexer, &rhs, precedence[bop].right);
        crsI_binary(lexer->chunk, exp, &rhs, bop, pc);
        bop = next;
    }

    return bop; /* consumed but not evaluated */
}

/* <name_list> ::= <name> ("," <name>)* */
static unsigned name_list(Lexer* lexer) {
    unsigned count = 0;

    do {
        crsI_local(lexer->chunk, get_name(lexer));
        count++;
    } while (check(lexer, ','));

    return count;
}

/*
 * ===========================
 *  blocks
 * ===========================
 */

/* <block_cond> ::= "(" <expr> ")" */
static void block_cond(Lexer* lexer, int test) {
    Expression exp;

    check_expected(lexer, '(');
    expr(lexer, &exp, 0);
    check_expected(lexer, ')');

    crsI_test(lexer->chunk, &exp, test);
    crsI_freeExp(lexer->chunk, &exp);
}

/* <block_body> ::= "{" <body> "}" */
static void block_body(Lexer* lexer) {
    check_expected(lexer, '{');
    body(lexer);
    check_expected(lexer, '}');
}

/*
 * ===========================
 *  functions
 * ===========================
 */

/* <func_name> ::= <name> <index>* */
static void func_name(Lexer* lexer, Expression* exp) {
    crsI_var(lexer->chunk, get_name(lexer), exp);
    while (index(lexer, exp));
}

/* <func_pars> ::= "(" <name_list>? ")" */
static void func_pars(Lexer* lexer) {
    check_expected(lexer, '(');

    if (!check(lexer, ')')) {
        /* values of parameters are given by the caller */
        crs_byte count           = (crs_byte)name_list(lexer);
        lexer->chunk->func->args = count;
        lexer->chunk->func->top  = count;
        lexer->chunk->regs      += count;

        check_expected(lexer, ')');
    }
}

/* <func_body> ::= <func_pars> <block_body> */
static void func_body(Lexer* lexer, Expression* exp) {
    Chunk* parent = lexer->chunk;
    Chunk  chunk;
    Scope  scope;

    exp->type    = EXP_FUNC;
    exp->value.v = crsI_nested(parent, &chunk);
    lexer->chunk = &chunk;

    crsI_enter(&chunk, &scope, 0);
    func_pars(lexer);
    block_body(lexer);
    crsI_leave(&chunk);
    crsI_finish(&chunk);

    lexer->chunk = parent;
}

/*
 * ===========================
 *  conditional statements
 * ===========================
 */

static void if_if(Lexer* lexer, unsigned* end) {
    Scope    scope;
    Chunk*   chunk = lexer->chunk;
    unsigned next  = PATCH_NONE;

    block_cond(lexer, 1);    /* if condition is false... */
    crsI_jump(chunk, &next); /* try next branch          */

    crsI_enter(chunk, &scope, 0);
    block_body(lexer);
    crsI_jump(chunk, end); /* goto end of statement */
    crsI_leave(chunk);

    crsI_patchHere(chunk, next); /* next branch (or end) starts here */
}

static void if_else(Lexer* lexer) {
    Scope  scope;
    Chunk* chunk = lexer->chunk;

    crsI_enter(chunk, &scope, 0);
    block_body(lexer);
    crsI_leave(chunk);
}

/*
 * <stat_if> ::= "if" <block_cond> <block_body>
 *               ("else" "if" <block_cond> <block_body>)*
 *               ("else" <block_body>)?
 */
static void stat_if(Lexer* lexer) {
    unsigned end = PATCH_NONE;

    crsL_next(lexer); /* consume 'if' */
    if_if(lexer, &end);

    while (check(lexer, TK_ELSE)) {
        if (check(lexer, TK_IF)) {
            if_if(lexer, &end);
        } else {
            if_else(lexer);
            break;
        }
    }

    crsI_patchHere(lexer->chunk, end); /* end of if statement */
}

/*
 * FIXME: actually finish the for loops i don't wanna do it right now so fuck
 *        you i guess hey its me its verity ask me anything i got a question
 */

/* <for_range> ::= <name> "=" <expr> "," <expr> ("," <expr>)? */
static void for_range(Lexer* lexer) {
    crsI_error(lexer->chunk, ">:(");
}

/* <stat_for> ::= "for" "(" <for_range> ")" <block_body> */
static void stat_for(Lexer* lexer) {
    for_range(lexer);
}

/* <stat_while> ::= "while" <block_cond> <block_body> */
static void stat_while(Lexer* lexer) {
    Scope  scope;
    Chunk* chunk = lexer->chunk;

    crsL_next(lexer); /* consume 'while' */
    crsI_enter(chunk, &scope, 1);

    crsI_label(chunk, lexer->names.cont); /* start of loop            */
    block_cond(lexer, 1);                 /* if condition is false... */
    crsI_goto(chunk, lexer->names.brk);   /* break out of loop        */

    block_body(lexer);
    crsI_goto(chunk, lexer->names.cont); /* try condition again */
    crsI_label(chunk, lexer->names.brk); /* end of loop         */

    crsI_patchAll(chunk);
    crsI_leave(chunk);
}

/* <stat_do> ::= "do" <block_body> "while" <block_cond> */
static void stat_do(Lexer* lexer) {
    Scope    scope;
    Chunk*   chunk = lexer->chunk;
    unsigned start = crsI_nextPC(chunk);

    crsL_next(lexer); /* consume 'do' */
    crsI_enter(chunk, &scope, 1);

    block_body(lexer);

    check_expected(lexer, TK_WHILE);
    crsI_label(chunk, lexer->names.cont); /* continue here           */
    block_cond(lexer, 0);                 /* if condition is true... */
    crsI_jumpTo(chunk, start);            /* restart loop            */
    crsI_label(chunk, lexer->names.brk);  /* end of loop             */

    crsI_patchAll(chunk);
    crsI_leave(chunk);
}

/*
 * ===========================
 *  control flow
 * ===========================
 */

static void loopGoto(Lexer* lexer, crs_String* label) {
    Chunk* chunk = lexer->chunk;

    if (!chunk->scope->inLoop) {
        crsL_error(lexer, "'%s' outside loop", label->contents);
    } else {
        crsI_goto(chunk, label);
    }
}

/* <stat_cont> ::= "continue" */
static void stat_cont(Lexer* lexer) {
    crsL_next(lexer); /* consume 'continue' */
    loopGoto(lexer, lexer->names.cont);
}

/* <stat_break> ::= "break" */
static void stat_break(Lexer* lexer) {
    crsL_next(lexer); /* consume 'break' */
    loopGoto(lexer, lexer->names.brk);
}

/* <stat_ret> ::= "return" <expr_list>? */
static void stat_ret(Lexer* lexer) {
    Expression exp;

    crsL_next(lexer); /* consume 'return' */
    crsL_peek(lexer);

    switch (lexer->peek.type) {
        case ';': case '}': case TK_EOF:
            exp.type = EXP_VOID;
            break;
        default:
            expr_list(lexer, &exp, 0);
    }

    crsI_return(lexer->chunk, &exp); /* 'crsI_return' frees list */
}

/*
 * ===========================
 *  declarations
 * ===========================
 */

/* <stat_local> ::= "local" <name_list> ("=" <expr_list>)? */
static void stat_local(Lexer* lexer) {
    Chunk*     chunk  = lexer->chunk;
    unsigned   vars   = name_list(lexer);
    unsigned   values = 0;
    Expression list;

    /* they aren't actually declared yet, so don't reference them */
    chunk->lV -= vars;

    if (check(lexer, '=')) {
        values = expr_list(lexer, &list, vars);

        if (list.type != EXP_LIST) {
            crsI_toTop(chunk, &list);
        } else if (values > vars) {
            values = vars;
        }
    }

    /* set missing values to nil */
    for (unsigned i = 0; i < vars - values; i++) {
        Expression nil = {.type = EXP_NIL};
        Expression var = {
            .type  = EXP_LOCAL,
            .value = {
                .v = chunk->regs + i
            }
        };

        crsI_assign(chunk, &var, &nil);
        crsI_freeExp(chunk, &nil);
    }

    crsI_freeExp(chunk, &list);
    chunk->regs = chunk->locals; /* now, they hold values, so... */
    chunk->lV  += vars;          /* they can be referenced */
}

/* <stat_localfunc> ::= "local" "function" <name> <func_body> */
static void stat_localfunc(Lexer* lexer) {
    Chunk*     chunk = lexer->chunk;
    Expression func;

    crsL_next(lexer); /* consume 'function' */

    crsI_local(chunk, get_name(lexer));
    func_body(lexer, &func);
    crsI_toTop(chunk, &func); /* allocate the register holding the local */
}

/* <stat_func> ::= "function" <func_name> <func_body> */
static void stat_func(Lexer* lexer) {
    Chunk*     chunk = lexer->chunk;
    Expression exp;
    Expression func;

    crsL_next(lexer); /* consume 'function' */
    func_name(lexer, &exp);
    func_body(lexer, &func);
    crsI_assign(chunk, &exp, &func);
    crsI_freeExp(chunk, &exp);
    crsI_freeExp(chunk, &func);
}

/*
 * ===========================
 *  assignments & expression
 *  statements
 * ===========================
 */

/*
 * - statement must have an effect:
 *   - single expression with side effects
 *   - assignment
 * - all expressions in an assignment must be assignable
 * - multiple expressions must be followed by an assignment
 */
static void doExprStat(Lexer* lexer, SubexpList* prev, unsigned count) {
    SubexpList exps;
    int        more;

    expr_sub(lexer, &exps.exp);
    more      = check(lexer, ',');
    exps.prev = prev;

    /* if there are multiple expressions */
    if (count++ || more) {
        if (!exp_assignable(&exps.exp)) {
            crsL_error(lexer, "expression is not assignable");
        } else if (more) {
            doExprStat(lexer, &exps, count);
            crsI_freeExp(lexer->chunk, &exps.exp);
            return;
        } /* otherwise, final expression in list; assignment expected */
    } /* otherwise, just one expression */

    if (!check(lexer, '=')) {
        if (count > 1) {
            crsL_next(lexer); /* consume unexpected token */
            crsL_expected(lexer, '=');
        } else if (!exp_haseffect(&exps.exp)) {
            crsL_error(lexer, "statement has no effect");
        } /* otherwise, single expression with side effects */

        crsI_freeExp(lexer->chunk, &exps.exp);
        return;
    } /* otherwise, do assignment */

    Expression values;
    expr_list(lexer, &values, count);

    crsI_multiAssign(lexer->chunk, &exps, &values, count);
    crsI_freeExp(lexer->chunk, &values);
    crsI_freeExp(lexer->chunk, &exps.exp); /* last exp isn't freed */
}

/* <stat_expr> ::= <expr_sub> ("," <expr_sub>)* ("=" <expr_list>)? */
static void stat_expr(Lexer* lexer) {
    doExprStat(lexer, NULL, 0);
}

/*
 * ===========================
 *  body
 * ===========================
 */

/*
 * <stat> ::= <stat_if>
 *          | <stat_for>
 *          | <stat_while>
 *          | <stat_do>
 *          | <stat_cont>
 *          | <stat_break>
 *          | <stat_ret>
 *          | <stat_local>
 *          | <stat_localfunc>
 *          | <stat_func>
 *          | <stat_expr>
 *          | ";"
 */
static int stat(Lexer* lexer) {
    /*
     * For the most part, the grammar of Crescent is unambiguous, save for the
     * following two cases:
     * - 'return' followed by <stat_expr>:
     *   return
     *   foo() // parsed as 'return foo()'
     * - <stat_expr> followed by '(':
     *   foo = bar
     *   (baz).x = quux // parsed as 'foo = bar(baz).z = quux' (invalid syntax)
     *
     * The lexer discards most whitespace, only using it as a separator for some
     * tokens that require it, so the parser simply sees the token immediately
     * following the first line with no knowledge of the whitespace nor the
     * intent of the programmer (which no language, to my knowledge, can ever
     * know :p).
     *
     * To address the first issue (which can be solved simply with a semicolon,
     * but this is mostly to prevent unexpected behavior), any return statement
     * in a block must either be followed by the end of the block or a
     * semicolon, such that anything else following it must therefore be part of
     * the statement.
     *
     * The second issue shouldn't be too common, but it can be solved by placing
     * a semicolon after the first assignment:
     *   foo = bar;
     *   (baz).x = quux
     */

    crsL_peek(lexer);
    int more = 1;

    switch (lexer->peek.type) {
        case TK_IF:
            stat_if(lexer);
            break;
        case TK_FOR:
            stat_for(lexer);
            break;
        case TK_WHILE:
            stat_while(lexer);
            break;
        case TK_DO:
            stat_do(lexer);
            break;
        case TK_CONT:
            stat_cont(lexer);
            break;
        case TK_BREAK:
            stat_break(lexer);
            break;
        case TK_RETURN:
            stat_ret(lexer);
            more = 0;          /* must be last statement in block... */
            check(lexer, ';'); /* save for the empty statement       */
            break;
        case TK_LOCAL:
            crsL_next(lexer); /* consume 'local' */
            crsL_peek(lexer);

            if (lexer->peek.type == TK_FUNC) {
                stat_localfunc(lexer);
            } else {
                stat_local(lexer);
            }

            break;
        case TK_FUNC:
            stat_func(lexer);
            break;
        case ';': /* empty statement */
            crsL_next(lexer); /* consume ';' */
            break;
        case '}': case TK_EOF: /* end of block */
            more = 0;
            break;
        default:
            stat_expr(lexer);
            break;
    }

    assert(lexer->chunk->regs == lexer->chunk->locals);
    return more;
}

/* <body> ::= <stat>* */
static void body(Lexer* lexer) {
    while (stat(lexer));
}

/*
 * ===========================
 *  compiler
 * ===========================
 */

static crs_Function* mainFunc(Lexer* lexer, Parser* parser) {
    Chunk       chunk;
    Scope       scope;
    crs_Thread* thread = lexer->thread;

    crs_Function* func = crsI_newChunk(&chunk, thread, parser);
    lexer->chunk       = &chunk;
    crsC_anchor(thread, obj_toheader(func));

    /* now it can be safely created and referenced */
    func->debug.source = crsS_new(thread, lexer->stream->source);

    crsI_enter(&chunk, &scope, 0);
    body(lexer);
    check_expected(lexer, TK_EOF);
    crsI_leave(&chunk);

    /* main function is unanchored by 'crsP_compile' */
    crsI_finish(&chunk);
    return func;
}

typedef struct {
    crs_Stream* stream;
    crs_Buffer* buffer;
    Lexer*      lexer;
    Parser*     parser;
} ParserInfo;

static void* compile(crs_Thread* thread, void* data) {
    ParserInfo* info   = data;
    Lexer*      lexer  = info->lexer;
    Parser*     parser = info->parser;

    crsB_init(thread, info->buffer);
    crsI_init(thread, parser, info->stream);
    crsL_init(thread, lexer, info->buffer, info->stream);

    return mainFunc(lexer, parser);
}

crs_Function* crsP_compile(crs_Stream* stream) {
    crs_Buffer  buffer;
    Lexer       lexer;
    Parser      parser;
    ParserInfo  info = {
        .stream = stream,
        .buffer = &buffer,
        .lexer  = &lexer,
        .parser = &parser
    };

    crs_Function* main;
    crs_Thread*   thread = stream->thread;
    int           status = crsC_try(thread, &compile, &info, (void**)&main);

    crsB_free(&buffer);
    crsI_free(thread, &parser);

    if (status != CRS_OK) {
        /* rethrow for 'tryLoad' */
        crsC_throw(thread, status); /* error is already in error register */
    } else {
        /* unanchor main function and string table */
        crsC_unanchor(thread);
        crsC_unanchor(thread);
    }

    return main;
}
