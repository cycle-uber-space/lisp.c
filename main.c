
#include "lisp.h"

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
    for (U64 i = 0; i < symbol->num; i++)
    {
        LISP_FREE(symbol->names[i]);
    }
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

char const * lisp_symbol_name(SymbolState * symbol, Expr exp)
{
    LISP_ASSERT(is_symbol(exp));
    U64 const index = expr_data(exp);
    if (index >= symbol->num)
    {
        LISP_FAIL("illegal symbol index %" PRIu64 "\n", index);
    }
    return symbol->names[index];
}

#if LISP_GLOBAL_API
Expr make_symbol(char const * name)
{
    return lisp_make_symbol(&global.symbol, name);
}

char const * symbol_name(Expr exp)
{
#if LISP_SYMBOL_NAME_OF_NIL
    if (exp == nil)
    {
        return "nil";
    }
#endif
    return lisp_symbol_name(&global.symbol, exp);
}

#endif

/* cons.c */

static void _cons_realloc(ConsState * cons)
{
    cons->pairs = (struct Pair *) LISP_REALLOC(cons->pairs, sizeof(struct Pair) * cons->max);
    if (!cons->pairs)
    {
        LISP_FAIL("cons memory allocation failed\n");
    }
}

static void _cons_maybe_realloc(ConsState * cons)
{
    if (cons->num < cons->max)
    {
        return;
    }

    if (LISP_MAX_CONSES == -1 || cons->max * 2 <= (U64) LISP_MAX_CONSES)
    {
        if (cons->max == 0)
        {
            cons->max = LISP_DEF_CONSES;
        }
        else
        {
            cons->max *= 2;
        }

        _cons_realloc(cons);
        return;
    }

    LISP_FAIL("cons ran over memory budget\n");
}

static struct Pair * _cons_lookup(ConsState * cons, U64 index)
{
    LISP_ASSERT_DEBUG(index < cons->num);
    return &cons->pairs[index];
}

void cons_init(ConsState * cons)
{
    memset(cons, 0, sizeof(ConsState));
}

void cons_quit(ConsState * cons)
{
}

bool is_cons(Expr exp)
{
    return expr_type(exp) == TYPE_CONS;
}

Expr lisp_cons(ConsState * cons, Expr a, Expr b)
{
    _cons_maybe_realloc(cons);

    U64 const index = cons->num++;
    struct Pair * pair = _cons_lookup(cons, index);
    pair->a = a;
    pair->b = b;
    return make_expr(TYPE_CONS, index);
}

Expr lisp_car(ConsState * cons, Expr exp)
{
    LISP_ASSERT(is_cons(exp));

    U64 const index = expr_data(exp);
    struct Pair * pair = _cons_lookup(cons, index);
    return pair->a;
}

Expr lisp_cdr(ConsState * cons, Expr exp)
{
    LISP_ASSERT(is_cons(exp));

    U64 const index = expr_data(exp);
    struct Pair * pair = _cons_lookup(cons, index);
    return pair->b;
}

void lisp_rplaca(ConsState * cons, Expr exp, Expr val)
{
    LISP_ASSERT(is_cons(exp));

    U64 const index = expr_data(exp);
    struct Pair * pair = _cons_lookup(cons, index);
    pair->a = val;
}

void lisp_rplacd(ConsState * cons, Expr exp, Expr val)
{
    LISP_ASSERT(is_cons(exp));

    U64 const index = expr_data(exp);
    struct Pair * pair = _cons_lookup(cons, index);
    pair->b = val;
}

Expr cons(Expr a, Expr b)
{
    return lisp_cons(&global.cons, a, b);
}

Expr car(Expr exp)
{
    return lisp_car(&global.cons, exp);
}

Expr cdr(Expr exp)
{
    return lisp_cdr(&global.cons, exp);
}

void rplaca(Expr exp, Expr val)
{
    lisp_rplaca(&global.cons, exp, val);
}

void rplacd(Expr exp, Expr val)
{
    lisp_rplacd(&global.cons, exp, val);
}

Expr caar(Expr exp)
{
    return car(car(exp));
}

Expr cdar(Expr exp)
{
    return cdr(car(exp));
}

/* stream.c */

void stream_init(StreamState * stream)
{
    memset(stream, 0, sizeof(StreamState));

    stream->stdin = lisp_make_file_input_stream(stream, stdin, false);
    stream->stdout = lisp_make_file_output_stream(stream, stdout, false);
    stream->stderr = lisp_make_file_output_stream(stream, stderr, false);
}

void stream_quit(StreamState * stream)
{
    for (U64 i = 0; i < stream->num; i++)
    {
        StreamInfo * info = stream->info + i;
        if (info->close_on_quit)
        {
            fclose(info->file);
        }
    }
}

bool is_stream(Expr exp)
{
    return expr_type(exp) == TYPE_STREAM;
}

static Expr _make_file_stream(StreamState * stream, FILE * file, bool close_on_quit)
{
    LISP_ASSERT(stream->num < LISP_MAX_STREAMS);
    U64 const index = stream->num++;
    StreamInfo * info = stream->info + index;
    memset(info, 0, sizeof(StreamInfo));
    info->file = file;
    info->close_on_quit = close_on_quit;
    return make_expr(TYPE_STREAM, index);
}

static void lisp_stream_show_info(StreamState * stream)
{
    for (U64 i = 0; i < stream->num; i++)
    {
        StreamInfo * info = stream->info + i;
        fprintf(stderr, "stream %d:\n", (int) i);
        fprintf(stderr, "- file: %p\n", info->file);
        fprintf(stderr, "- buffer: %p\n", info->buffer);
    }
}

Expr lisp_make_file_input_stream(StreamState * stream, FILE * file, bool close_on_quit)
{
    return _make_file_stream(stream, file, close_on_quit);
}

Expr lisp_make_file_output_stream(StreamState * stream, FILE * file, bool close_on_quit)
{
    return _make_file_stream(stream, file, close_on_quit);
}

Expr lisp_make_buffer_output_stream(StreamState * stream, size_t size, char * buffer)
{
    LISP_ASSERT(stream->num < LISP_MAX_STREAMS);
    U64 const index = stream->num++;
    StreamInfo * info = stream->info + index;
    memset(info, 0, sizeof(StreamInfo));
    info->size = size;
    info->buffer = buffer;
    info->cursor = 0;

    return make_expr(TYPE_STREAM, index);
}

void lisp_stream_put_string(StreamState * stream, Expr exp, char const * str)
{
    LISP_ASSERT(is_stream(exp));
    U64 const index = expr_data(exp);
    LISP_ASSERT(index < stream->num);
    StreamInfo * info = stream->info + index;
    if (info->file)
    {
        fputs(str, info->file);
    }

    if (info->buffer)
    {
        size_t const len = strlen(str);
        LISP_ASSERT(info->cursor + len + 1 < info->size);
        memcpy(info->buffer + info->cursor, str, len + 1);
        info->cursor += len;
    }
}

void lisp_stream_release(StreamState * stream, Expr exp)
{
    LISP_ASSERT(is_stream(exp));
    U64 const index = expr_data(exp);
    LISP_ASSERT(index < stream->num);
    StreamInfo * info = stream->info + index;
    if (info->close_on_quit)
    {
        fclose(info->file);
    }
    memcpy(info, stream->info + --stream->num, sizeof(StreamInfo));
}

void stream_put_string(Expr exp, char const * str)
{
    lisp_stream_put_string(&global.stream, exp, str);
}

void stream_release(Expr exp)
{
    lisp_stream_release(&global.stream, exp);
}

/* printer.c */

void render_expr(Expr exp, Expr out);

void render_expr(Expr exp, Expr out)
{
    switch (expr_type(exp))
    {
    case TYPE_NIL:
        LISP_ASSERT_DEBUG(expr_data(exp) == 0);
        stream_put_string(out, "nil");
        break;
    case TYPE_SYMBOL:
        stream_put_string(out, symbol_name(exp));
        break;
    default:
        LISP_FAIL("cannot print expression %016" PRIx64 "\n", exp);
        break;
    }
}

/* util.c */

char const * repr(Expr exp)
{
    // TODO multiple calls => need temp buffer per call
    static char buffer[4096] = { 0 };
    Expr out = lisp_make_buffer_output_stream(&global.stream, 4096, buffer);
    render_expr(exp, out);
    stream_release(out);
    return buffer;
}

void println(Expr exp)
{
    Expr out = global.stream.stdout;
    render_expr(exp, out);
    stream_put_string(out, "\n");
}

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

Expr list_1(Expr exp1)
{
    return cons(exp1, nil);
}

Expr list_2(Expr exp1, Expr exp2)
{
    return cons(exp1, cons(exp2, nil));
}

Expr first(Expr seq)
{
    return car(seq);
}

Expr second(Expr seq)
{
    return car(cdr(seq));
}

/* env.c */

static Expr env_vars(Expr env);
static void env_set_vars(Expr env, Expr vars);
static Expr env_vals(Expr env);
static void env_set_vals(Expr env, Expr vals);
static Expr env_outer(Expr env);

static Expr _env_find_local(Expr env, Expr var)
{
    Expr vars = env_vars(env);
    Expr vals = env_vals(env);
    while (vars)
    {
        if (car(vars) == var)
        {
            return vals;
        }
        vars = cdr(vars);
        vals = cdr(vals);
    }
    return nil;
}

static Expr _env_find_global(Expr env, Expr var)
{
    while (env)
    {
        Expr const vals = _env_find_local(env, var);
        if (vals)
        {
            return vals;
        }
        else
        {
            env = env_outer(env);
        }
    }
    return nil;
}

Expr make_env(Expr outer)
{
    // ((<vars> . <vals>) . <outer>)
    // TODO add dummy conses as sentinels for vars and vals
    return cons(cons(nil, nil), outer);
}

static Expr env_vars(Expr env)
{
    return caar(env);
}

static void env_set_vars(Expr env, Expr vars)
{
    rplaca(car(env), vars);
}

static Expr env_vals(Expr env)
{
    return cdar(env);
}

static void env_set_vals(Expr env, Expr vals)
{
    rplacd(car(env), vals);
}

static Expr env_outer(Expr env)
{
    return cdr(env);
}

void env_def(Expr env, Expr var, Expr val)
{
    Expr const vals = _env_find_local(env, var);
    if (vals)
    {
        rplaca(vals, val);
    }
    else
    {
        env_set_vars(env, cons(var, env_vars(env)));
        env_set_vals(env, cons(val, env_vals(env)));
    }
}

void env_del(Expr env, Expr var)
{
    Expr prev_vars = nil;
    Expr prev_vals = nil;

    Expr vars = env_vars(env);
    Expr vals = env_vals(env);
    while (vars)
    {
        if (car(vars) == var)
        {
            if (prev_vars)
            {
                LISP_ASSERT(prev_vals);
                rplacd(prev_vars, cdr(vars));
                rplacd(prev_vals, cdr(vals));
            }
            else
            {
                env_set_vars(env, cdr(vars));
                env_set_vals(env, cdr(vals));
            }
            return;
        }

        prev_vars = vars;
        prev_vals = vals;
        vars = cdr(vars);
        vals = cdr(vals);
    }

    LISP_FAIL("unbound variable %s\n", repr(var));
}

bool env_can_set(Expr env, Expr var)
{
    Expr const tmp = _env_find_global(env, var);
    return tmp != nil;
}

Expr env_get(Expr env, Expr var)
{
    Expr const vals = _env_find_global(env, var);
    if (vals)
    {
        return car(vals);
    }
    else
    {
        LISP_FAIL("unbound variable %s\n", repr(var));
        return nil;
    }
}

void env_set(Expr env, Expr var, Expr val)
{
    Expr const vals = _env_find_global(env, var);
    if (vals)
    {
        rplaca(vals, val);
    }
    else
    {
        LISP_FAIL("unbound variable %s\n", repr(var));
    }
}

/* core.c */

Expr make_core_env()
{
    Expr env = make_env(nil);
    env_def(env, intern("t"), intern("t"));
    return env;
}

/* eval.c */

Expr eval(Expr exp, Expr env)
{
    //fprintf(stderr, "EVAL %016" PRIx64 "\n", exp);
    if (exp == nil)
    {
        return nil;
    }

    switch (expr_type(exp))
    {
    case TYPE_SYMBOL:
        if (exp == intern("*env*"))
        {
            return env;
        }
        return env_get(env, exp);
    default:
        LISP_FAIL("cannot evaluate %s\n", repr(exp));
        return nil;
    }
}

/* system.c */

void system_init(SystemState * system)
{
    symbol_init(&system->symbol);
    cons_init(&system->cons);
    stream_init(&system->stream);
}

void system_quit(SystemState * system)
{
    stream_quit(&system->stream);
    cons_quit(&system->cons);
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

static void unit_test_symbol(TestState * test)
{
    LISP_TEST_GROUP(test, "symbol");
    LISP_TEST_ASSERT(test, !strcmp("foo", symbol_name(intern("foo"))));
    LISP_TEST_ASSERT(test, !strcmp("bar", symbol_name(intern("bar"))));
#if LISP_SYMBOL_NAME_OF_NIL
    LISP_TEST_ASSERT(test, !strcmp("nil", symbol_name(intern("nil"))));
#endif

    LISP_TEST_ASSERT(test, intern("foo") == intern("foo"));
    LISP_TEST_ASSERT(test, intern("foo") != intern("bar"));
}

static void unit_test_cons(TestState * test)
{
    LISP_TEST_GROUP(test, "cons");
    LISP_TEST_ASSERT(test, is_cons(cons(nil, nil)));
    LISP_TEST_ASSERT(test, car(cons(nil, nil)) == nil);
    LISP_TEST_ASSERT(test, cdr(cons(nil, nil)) == nil);
}

static void unit_test_stream(TestState * test)
{
    LISP_TEST_GROUP(test, "stream");
    LISP_TEST_ASSERT(test, is_stream(global.stream.stdin));
    LISP_TEST_ASSERT(test, is_stream(global.stream.stdout));
    LISP_TEST_ASSERT(test, is_stream(global.stream.stderr));
}

static void unit_test_printer(TestState * test)
{
    LISP_TEST_GROUP(test, "printer");
    LISP_TEST_ASSERT(test, !strcmp("nil", repr(nil)));
}

static void unit_test_util(TestState * test)
{
    LISP_TEST_GROUP(test, "util");
    LISP_TEST_ASSERT(test, intern("nil") == nil);
    LISP_TEST_ASSERT(test, is_nil(intern("nil")));
    LISP_TEST_ASSERT(test, is_symbol(intern("nul")));

    {
        Expr const foo = intern("foo");
        Expr const bar = intern("bar");
        LISP_TEST_ASSERT(test, first(list_1(foo)) == foo);
        LISP_TEST_ASSERT(test, first(list_2(foo, bar)) == foo);
        LISP_TEST_ASSERT(test, second(list_2(foo, bar)) == bar);
    }
}

static void unit_test_env(TestState * test)
{
    LISP_TEST_GROUP(test, "env");
    {
        Expr env = make_env(nil);
        Expr const foo = intern("foo");
        Expr const bar = intern("bar");
        LISP_TEST_ASSERT(test, !env_can_set(env, foo));
        LISP_TEST_ASSERT(test, !env_can_set(env, bar));
        env_def(env, foo, bar);
        LISP_TEST_ASSERT(test,  env_can_set(env, foo));
        LISP_TEST_ASSERT(test, !env_can_set(env, bar));
        LISP_TEST_ASSERT(test, env_get(env, foo) == bar);

        env_def(env, bar, foo);
        LISP_TEST_ASSERT(test,  env_can_set(env, foo));
        LISP_TEST_ASSERT(test,  env_can_set(env, bar));
        LISP_TEST_ASSERT(test, env_get(env, foo) == bar);
        LISP_TEST_ASSERT(test, env_get(env, bar) == foo);

        env_del(env, foo);
        LISP_TEST_ASSERT(test, !env_can_set(env, foo));
        LISP_TEST_ASSERT(test,  env_can_set(env, bar));
        LISP_TEST_ASSERT(test, env_get(env, bar) == foo);
    }
    {
        Expr env1 = make_env(nil);
        Expr env2 = make_env(env1);
        Expr const foo = intern("foo");
        Expr const bar = intern("bar");

        LISP_TEST_ASSERT(test, !env_can_set(env1, foo));
        LISP_TEST_ASSERT(test, !env_can_set(env2, foo));

        LISP_TEST_ASSERT(test, !env_can_set(env1, bar));
        LISP_TEST_ASSERT(test, !env_can_set(env2, bar));

        env_def(env1, foo, foo);

        LISP_TEST_ASSERT(test,  env_can_set(env1, foo));
        LISP_TEST_ASSERT(test,  env_can_set(env2, foo));
        LISP_TEST_ASSERT(test, !env_can_set(env2, bar));
    }
}

static void unit_test_eval(TestState * test)
{
    LISP_TEST_GROUP(test, "eval");
    LISP_TEST_ASSERT(test, eval(nil, nil) == nil);

    {
        Expr env = make_core_env();
        Expr t = intern("t");
        LISP_TEST_ASSERT(test, eval(t, env) == t);

        LISP_TEST_ASSERT(test, eval(intern("*env*"), env) == env);
    }
}

static void unit_test(TestState * test)
{
    LISP_TEST_BEGIN(test);
    unit_test_expr(test);
    unit_test_nil(test);
    unit_test_symbol(test);
    unit_test_cons(test);
    unit_test_stream(test);
    unit_test_printer(test);
    unit_test_util(test);
    unit_test_env(test);
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
