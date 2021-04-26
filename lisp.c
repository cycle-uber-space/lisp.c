
/* lisp.h */

#ifndef LISP_GLOBAL_API
#define LISP_GLOBAL_API 1
#endif

#ifndef LISP_DEBUG
#define LISP_DEBUG 1
#endif

#define LISP_MAX_SYMBOLS -1
#define LISP_DEF_SYMBOLS 16

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <assert.h>
#include <string.h>

#define LISP_RED     "\x1b[31m"
#define LISP_GREEN   "\x1b[32m"
#define LISP_YELLOW  "\x1b[33m"
#define LISP_BLUE    "\x1b[34m"
#define LISP_MAGENTA "\x1b[35m"
#define LISP_CYAN    "\x1b[36m"
#define LISP_RESET   "\x1b[0m"

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

typedef float F32;
typedef double F64;

static_assert(sizeof(U8 ) == 1, "");
static_assert(sizeof(U16) == 2, "");
static_assert(sizeof(U32) == 4, "");
static_assert(sizeof(U64) == 8, "");

static_assert(sizeof(I8 ) == 1, "");
static_assert(sizeof(I16) == 2, "");
static_assert(sizeof(I32) == 4, "");
static_assert(sizeof(I64) == 8, "");

static_assert(sizeof(F32) == 4, "");
static_assert(sizeof(F64) == 8, "");

#ifndef LISP_MALLOC
#define LISP_MALLOC(size) malloc(size)
#endif

#ifndef LISP_REALLOC
#define LISP_REALLOC(ptr, size) realloc(ptr, size)
#endif

#ifndef LISP_FREE
#define LISP_FREE(ptr) free(ptr)
#endif

/* test.h */

typedef struct
{
    int num_tests;
    int num_failed;
    bool show_pass;
    bool exit_on_fail;
} TestState;

#define LISP_TEST_GROUP(test, text) test_group(test, text)
#define LISP_TEST_BEGIN(test)       test_begin(test)
#define LISP_TEST_FINISH(test)      test_finish(test)
#define LISP_TEST_ASSERT(test, exp) do { test_assert_try(test, exp, #exp); } while (0)

void test_begin(TestState * test);
void test_finish(TestState * test);
void test_group(TestState * test, char const * text);
void test_assert_try(TestState * test, bool exp, char const * msg);

/* error.h */

#define LISP_FAIL(...)    error_fail(__VA_ARGS__);
#define LISP_WARN(...)    error_warn(__VA_ARGS__);

#define LISP_ASSERT(x) assert(x)

#if LISP_DEBUG
#define LISP_ASSERT_DEBUG(x) LISP_ASSERT(x)
#else
#define LISP_ASSERT_DEBUG(x)
#endif

void error_fail(char const * fmt, ...);
void error_warn(char const * fmt, ...);

/* expr.h */

typedef U64 Expr;

Expr make_expr(U64 type, U64 data);
U64 expr_type(Expr exp);
U64 expr_data(Expr exp);

enum
{
    TYPE_NIL = 0,
    TYPE_SYMBOL,
};

enum
{
    DATA_NIL = 0,
};

/* nil.h */

#define nil 0

inline static bool is_nil(Expr exp)
{
    return exp == nil;
}

/* symbol.h */

typedef struct
{
    U64 num;
    U64 max;
    char ** names;
} SymbolState;

void symbol_init(SymbolState * symbol);
void symbol_quit(SymbolState * symbol);

inline static bool is_symbol(Expr exp)
{
    return expr_type(exp) == TYPE_SYMBOL;
}

Expr lisp_make_symbol(SymbolState * symbol, char const * name);

#if LISP_GLOBAL_API
Expr make_symbol(char const * name);
#endif

/* util.h */

Expr intern(char const * name);

/* eval.h */

Expr eval(Expr exp, Expr env);

/* system.h */

typedef struct
{
    SymbolState symbol;
} SystemState;

void system_init(SystemState * system);
void system_quit(SystemState * system);

/* global.h */

extern SystemState global;

void global_init();
void global_quit();

/* test.c */

#define LISP_TEST_FILE stdout

void test_begin(TestState * test)
{
    test->num_tests    = 0;
    test->num_failed   = 0;
    test->show_pass    = true;
    test->exit_on_fail = false;
}

void test_finish(TestState * test)
{
    if (test->num_failed)
    {
        fprintf(LISP_TEST_FILE, LISP_RED "FAIL" LISP_RESET " " "%d/%d test(s)\n", test->num_failed, test->num_tests);
    }
    else
    {
        fprintf(LISP_TEST_FILE, LISP_GREEN "PASS" LISP_RESET " " "%d/%d test(s)\n", test->num_tests, test->num_tests);
    }
}

void test_group(TestState * test, char const * text)
{
    fprintf(LISP_TEST_FILE, "==== %s ====\n", text);
}

void test_assert_try(TestState * test, bool exp, char const * msg)
{
    ++test->num_tests;
    if (exp)
    {
        if (test->show_pass)
        {
            fprintf(LISP_TEST_FILE, LISP_GREEN "PASS" LISP_RESET " %s\n", msg);
        }
    }
    else
    {
        ++test->num_failed;
        fprintf(LISP_TEST_FILE, LISP_RED "FAIL" LISP_RESET " %s\n", msg);
        if (test->exit_on_fail)
        {
            exit(1);
        }
    }
}

/* error.c */

void error_fail(char const * fmt, ...)
{
    FILE * const file = stderr;
    va_list ap;
    va_start(ap, fmt);
    fprintf(file, LISP_RED "[FAIL] " LISP_RESET);
    vfprintf(file, fmt, ap);
    va_end(ap);
    exit(1);
}

void error_warn(char const * fmt, ...)
{
    FILE * const file = stderr;
    va_list ap;
    va_start(ap, fmt);
    fprintf(file, LISP_YELLOW "[WARN] " LISP_RESET);
    vfprintf(file, fmt, ap);
    va_end(ap);
}

/* expr.c */

typedef U64 Expr;

Expr make_expr(U64 type, U64 data)
{
    return (data << 8) | (type & 0xff);
}

U64 expr_type(Expr exp)
{
    return exp & 0xff;
}

U64 expr_data(Expr exp)
{
    return exp >> 8;
}

/* symbol.c */

static void _symbol_maybe_realloc(SymbolState * symbol)
{
    if (symbol->num < symbol->max)
    {
        return;
    }

    if (LISP_MAX_SYMBOLS == -1 || symbol->max * 2 <= LISP_MAX_SYMBOLS)
    {
        if (symbol->max == 0)
        {
            symbol->max = LISP_DEF_SYMBOLS;
        }
        else
        {
            symbol->max *= 2;
        }

        symbol->names = (char **) LISP_REALLOC(symbol->names, sizeof(char *) * symbol->max);
        if (!symbol->names)
        {
            LISP_FAIL("symbol memory allocation failed\n");
        }
        return;
    }

    LISP_FAIL("intern ran over memory budget\n");
}

void symbol_init(SymbolState * symbol)
{
    LISP_ASSERT_DEBUG(symbol);

    memset(symbol, 0, sizeof(SymbolState));
    _symbol_maybe_realloc(symbol);
}

void symbol_quit(SymbolState * symbol)
{
    LISP_FREE(symbol->names);
    memset(symbol, 0, sizeof(SymbolState));
}

Expr lisp_make_symbol(SymbolState * symbol, char const * name)
{
    LISP_ASSERT(name);
    size_t const len = strlen(name);

    for (U64 index = 0; index < symbol->num; ++index)
    {
        char const * str = symbol->names[index];
        size_t const tmp = strlen(str); // TODO cache this?
        if (len == tmp && !strncmp(name, str, len))
        {
            return make_expr(TYPE_SYMBOL, index);
        }
    }

    _symbol_maybe_realloc(symbol);

    U64 const index = symbol->num;
    char * buffer = (char *) LISP_MALLOC(len + 1);
    memcpy(buffer, name, len);
    buffer[len] = 0;
    symbol->names[index] = buffer;
    ++symbol->num;

    return make_expr(TYPE_SYMBOL, index);
}

#if LISP_GLOBAL_API
Expr make_symbol(char const * name)
{
    return lisp_make_symbol(&global.symbol, name);
}
#endif

/* util.c */

Expr intern(char const * name)
{
    if (!strcmp("nil", name))
    {
        return nil;
    }
    else
    {
        return make_symbol(name);
    }
}

/* eval.c */

Expr eval(Expr exp, Expr env)
{
    return nil;
}

/* system.c */

void system_init(SystemState * system)
{
    symbol_init(&system->symbol);
}

void system_quit(SystemState * system)
{
    symbol_quit(&system->symbol);
}

/* global.c */

SystemState global;

void global_init()
{
    system_init(&global);
}

void global_quit()
{
    system_quit(&global);
}

/* main.c */

static void fail(char const * fmt, ...)
{
    if (fmt)
    {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    fprintf(stderr,
            "usage: lisp <command> <options>\n"
            "commands:\n"
            "  unit .... run unit tests\n"
        );
    exit(1);
}

static void unit_test_expr(TestState * test)
{
    LISP_TEST_GROUP(test, "expr");
    LISP_TEST_ASSERT(test, expr_type(make_expr(23, 42)) == 23);
    LISP_TEST_ASSERT(test, expr_data(make_expr(23, 42)) == 42);
}

static void unit_test_nil(TestState * test)
{
    LISP_TEST_GROUP(test, "nil");
    LISP_TEST_ASSERT(test, expr_type(nil) == TYPE_NIL);
    LISP_TEST_ASSERT(test, expr_data(nil) == DATA_NIL);
    LISP_TEST_ASSERT(test, (bool) nil == false);
    LISP_TEST_ASSERT(test, is_nil(nil));
}

static void unit_test_util(TestState * test)
{
    LISP_TEST_GROUP(test, "util");
    LISP_TEST_ASSERT(test, intern("nil") == nil);
    LISP_TEST_ASSERT(test, is_nil(intern("nil")));
    LISP_TEST_ASSERT(test, is_symbol(intern("nul")));
}

static void unit_test_eval(TestState * test)
{
    LISP_TEST_GROUP(test, "eval");
    LISP_TEST_ASSERT(test, eval(nil, nil) == nil);
}

static void unit_test(TestState * test)
{
    LISP_TEST_BEGIN(test);
    unit_test_expr(test);
    unit_test_nil(test);
    unit_test_util(test);
    unit_test_eval(test);
    LISP_TEST_GROUP(test, "summary");
    LISP_TEST_FINISH(test);
}

int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        fail("missing command\n");
    }
    char const * cmd = argv[1];
    if (!strcmp("unit", cmd))
    {
        global_init();
        TestState test;
        unit_test(&test);
        global_quit();
    }
    else
    {
        fail("unknown command: %s\n", cmd);
    }
    return 0;
}
