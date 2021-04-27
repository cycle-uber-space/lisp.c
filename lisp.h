
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

#ifndef LISP_READER_PARSE_QUOTE
#define LISP_READER_PARSE_QUOTE 1
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
    TYPE_GENSYM,
    TYPE_STRING,
    TYPE_STREAM,
    TYPE_SPECIAL,
    TYPE_BUILTIN,
};

enum
{
    DATA_NIL = 0,
};

typedef struct SystemState SystemState;

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

#if LISP_GLOBAL_API

Expr cons(Expr a, Expr b);
Expr car(Expr exp);
Expr cdr(Expr exp);
void rplaca(Expr exp, Expr val);
void rplacd(Expr exp, Expr val);

inline static Expr caar(Expr exp)
{
    return car(car(exp));
}

inline static Expr cadr(Expr exp)
{
    return car(cdr(exp));
}

inline static Expr cdar(Expr exp)
{
    return cdr(car(exp));
}

inline static Expr cddr(Expr exp)
{
    return cdr(cdr(exp));
}

inline static Expr caddr(Expr exp)
{
    return car(cdr(cdr(exp)));
}

inline static Expr cdddr(Expr exp)
{
    return cdr(cdr(cdr(exp)));
}

inline static Expr cadddr(Expr exp)
{
    return car(cdr(cdr(cdr(exp))));
}

inline static Expr cddddr(Expr exp)
{
    return cdr(cdr(cdr(cdr(exp))));
}

#endif

/* gensym.h */

typedef struct
{
    U64 counter;
} GensymState;

void gensym_init(GensymState * gensym);
void gensym_quit(GensymState * gensym);

bool is_gensym(Expr exp);

Expr lisp_gensym(GensymState * gensym);

/* string.h */

#define LISP_MAX_STRINGS        500000

typedef struct
{
    U64 count;
    char ** values;
} StringState;

void string_init(StringState * string);
void string_quit(StringState * string);

bool is_string(Expr exp);

Expr make_string(char const * str);
char const * string_value(Expr exp);
U64 string_length(Expr exp);

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
Expr lisp_make_string_input_stream(StreamState * stream, char const * str);
Expr lisp_make_buffer_output_stream(StreamState * stream, size_t size, char * buffer);

bool lisp_stream_at_end(StreamState * stream, Expr exp);

void lisp_stream_release(StreamState * stream, Expr exp);

Expr make_file_input_stream_from_path(char const * path);
Expr make_string_input_stream(char const * str);

char stream_peek_char(Expr exp);
void stream_skip_char(Expr exp);

void stream_put_char(Expr exp, char ch);
void stream_put_string(Expr exp, char const * str);
void stream_put_u64(Expr exp, U64 val);

inline static char stream_get_char(Expr exp)
{
    /* TODO inline implementation */
    char const ret = stream_peek_char(exp);
    stream_skip_char(exp);
    return ret;
}

void stream_release(Expr exp);

/* special.h */

#define LISP_MAX_SPECIALS 64

typedef Expr (* SpecialFun)(Expr args, Expr kwargs, Expr env); // TODO should we pass the system state?

typedef struct
{
    char const * name;
    SpecialFun fun;
} SpecialInfo;

typedef struct
{
    U64 num;
    SpecialInfo info[LISP_MAX_SPECIALS];
} SpecialState;

void special_init(SpecialState * special);
void special_quit(SpecialState * special);

bool is_special(Expr exp);

Expr lisp_make_special(SpecialState * special, char const * name, SpecialFun fun);

char const * lisp_special_name(SpecialState * special, Expr exp);
SpecialFun lisp_special_fun(SpecialState * special, Expr exp);

Expr make_special(char const * name, SpecialFun fun);

/* builtin.h */

#define LISP_MAX_BUILTINS 64

typedef Expr (* BuiltinFun)(Expr args, Expr kwargs, Expr env); // TODO should we pass the system state?

typedef struct
{
    char const * name;
    BuiltinFun fun;
} BuiltinInfo;

typedef struct
{
    U64 num;
    BuiltinInfo info[LISP_MAX_BUILTINS];
} BuiltinState;

void builtin_init(BuiltinState * builtin);
void builtin_quit(BuiltinState * builtin);

bool is_builtin(Expr exp);

Expr lisp_make_builtin(BuiltinState * builtin, char const * name, BuiltinFun fun);

char const * lisp_builtin_name(BuiltinState * builtin, Expr exp);
BuiltinFun lisp_builtin_fun(BuiltinState * builtin, Expr exp);

Expr make_builtin(char const * name, BuiltinFun fun);

/* reader.h */

bool lisp_maybe_parse_expr(SystemState * system, Expr in, Expr * exp);

Expr lisp_read_one_from_string(SystemState * system, char const * src);

#if LISP_GLOBAL_API
bool maybe_parse_expr(Expr in, Expr * exp);
Expr read_one_from_string(char const * src);
#endif

/* printer.h */

void render_expr(Expr exp, Expr out);

/* util.h */

char * get_temp_buf(size_t size);

inline static bool eq(Expr a, Expr b)
{
    return a == b;
}

bool equal(Expr a, Expr b);

char const * repr(Expr exp);
void println(Expr exp);

Expr intern(char const * name);

Expr list_1(Expr exp1);
Expr list_2(Expr exp1, Expr exp2);
Expr list_3(Expr exp1, Expr exp2, Expr exp3);

Expr first(Expr seq);
Expr second(Expr seq);

Expr nreverse(Expr seq);
Expr append(Expr seq1, Expr seq2);

/* env.h */

Expr make_env(Expr outer);

void env_def(Expr env, Expr var, Expr val);
void env_del(Expr env, Expr var);

bool env_can_set(Expr env, Expr var);

Expr env_get(Expr env, Expr var);
void env_set(Expr env, Expr var, Expr val);

void env_destructuring_bind(Expr env, Expr vars, Expr vals);

/* core.h */

Expr make_core_env();

/* eval.h */

Expr eval(Expr exp, Expr env);

/* system.h */

typedef struct SystemState
{
    SymbolState symbol;
    ConsState cons;
    StreamState stream;
    GensymState gensym;
    StringState string;
    SpecialState special;
    BuiltinState builtin;
} SystemState;

void system_init(SystemState * system);
void system_quit(SystemState * system);

void load_file(char const * path, Expr env);

/* global.h */

#if LISP_GLOBAL_API

extern SystemState global;

void global_init();
void global_quit();

inline static bool stream_at_end(Expr exp)
{
    return lisp_stream_at_end(&global.stream, exp);
}

inline static char const * builtin_name(Expr exp)
{
    return lisp_builtin_name(&global.builtin, exp);
}

inline static BuiltinFun builtin_fun(Expr exp)
{
    return lisp_builtin_fun(&global.builtin, exp);
}

inline static char const * special_name(Expr exp)
{
    return lisp_special_name(&global.special, exp);
}

inline static SpecialFun special_fun(Expr exp)
{
    return lisp_special_fun(&global.special, exp);
}

#endif

#endif /* _LISP_H_ */
