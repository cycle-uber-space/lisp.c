
#ifndef _LISP_H_
#define _LISP_H_

#ifndef LISP_GLOBAL_API
#define LISP_GLOBAL_API 1
#endif

#ifndef LISP_DEBUG
#define LISP_DEBUG 1
#endif

#ifndef LISP_SYMBOL_NAME_OF_NIL
#define LISP_SYMBOL_NAME_OF_NIL 1
#endif

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <assert.h>
#include <inttypes.h>
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
    TYPE_CONS,
    TYPE_STREAM,
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

#define LISP_MAX_SYMBOLS -1
#define LISP_DEF_SYMBOLS 16

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
char const * lisp_symbol_name(SymbolState * symbol, Expr exp);

#if LISP_GLOBAL_API
Expr make_symbol(char const * name);
char const * symbol_name(Expr exp);
#endif

/* cons.h */

#define LISP_MAX_CONSES -1
#define LISP_DEF_CONSES  4

struct Pair
{
    Expr a, b;
};

typedef struct
{
    U64 num;
    U64 max;
    struct Pair * pairs;
} ConsState;

void cons_init(ConsState * cons);
void cons_quit(ConsState * cons);

bool is_cons(Expr exp);

Expr lisp_cons(ConsState * cons, Expr a, Expr b);
Expr lisp_car(ConsState * cons, Expr exp);
Expr lisp_cdr(ConsState * cons, Expr exp);
void lisp_rplaca(ConsState * cons, Expr exp, Expr val);
void lisp_rplacd(ConsState * cons, Expr exp, Expr val);

Expr cons(Expr a, Expr b);
Expr car(Expr exp);
Expr cdr(Expr exp);

void rplaca(Expr exp, Expr val);
void rplacd(Expr exp, Expr val);

inline static Expr caar(Expr exp)
{
    return car(car(exp));
}

inline static Expr cdar(Expr exp)
{
    return cdr(car(exp));
}

/* stream.h */

#define LISP_MAX_STREAMS 64

typedef struct
{
    FILE * file;
    bool close_on_quit;

    char * buffer;
    size_t size;
    size_t cursor;
} StreamInfo;

typedef struct
{
    U64 num;
    StreamInfo info[LISP_MAX_STREAMS];

    Expr stdin;
    Expr stdout;
    Expr stderr;
} StreamState;

void stream_init(StreamState * stream);
void stream_quit(StreamState * stream);

bool is_stream(Expr exp);

Expr lisp_make_file_input_stream(StreamState * stream, FILE * file, bool close_on_quit);
Expr lisp_make_file_output_stream(StreamState * stream, FILE * file, bool close_on_quit);

Expr lisp_make_buffer_output_stream(StreamState * stream, size_t size, char * buffer);

Expr make_string_input_stream(char const * str);

void stream_put_string(Expr exp, char const * str);

void stream_release(Expr exp);

/* printer.h */

void render_expr(Expr exp, Expr out);

/* util.h */

char const * repr(Expr exp);

Expr intern(char const * name);

Expr list_1(Expr exp1);
Expr list_2(Expr exp1, Expr exp2);

Expr first(Expr seq);
Expr second(Expr seq);

/* env.h */

Expr make_env(Expr outer);

void env_def(Expr env, Expr var, Expr val);
void env_del(Expr env, Expr var);

bool env_can_set(Expr env, Expr var);

Expr env_get(Expr env, Expr var);
void env_set(Expr env, Expr var, Expr val);

/* core.h */

Expr make_core_env();

/* eval.h */

Expr eval(Expr exp, Expr env);

/* system.h */

typedef struct
{
    SymbolState symbol;
    ConsState cons;
    StreamState stream;
} SystemState;

void system_init(SystemState * system);
void system_quit(SystemState * system);

/* global.h */

extern SystemState global;

void global_init();
void global_quit();

#endif /* _LISP_H_ */
