
#include "common.h"

#define TEMP_BUF_SIZE  4096
#define TEMP_BUF_COUNT 4

bool is_named_call(Expr exp, Expr name)
{
    return is_cons(exp) && eq(car(exp), name);
}

char * get_temp_buf(size_t size)
{
    LISP_ASSERT(size <= TEMP_BUF_SIZE);

    static char buf[TEMP_BUF_COUNT][TEMP_BUF_SIZE];
    static int idx = 0;
    char * ret = buf[idx];
    idx = (idx + 1) % TEMP_BUF_COUNT;
    return ret;
}

bool equal(Expr a, Expr b)
{
    if (is_cons(a) && is_cons(b))
    {
        return equal(car(a), car(b)) && equal(cdr(a), cdr(b));
    }
    return eq(a, b);
}

char const * repr(Expr exp)
{
    // TODO multiple calls => need temp buffer per call
    size_t const size = 4096;
    char * buffer = get_temp_buf(size);
    Expr out = lisp_make_buffer_output_stream(&global.stream, size, buffer);
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

Expr list_3(Expr exp1, Expr exp2, Expr exp3)
{
    return cons(exp1, cons(exp2, cons(exp3, nil)));
}

Expr first(Expr seq)
{
    return car(seq);
}

Expr second(Expr seq)
{
    return car(cdr(seq));
}

Expr nreverse(Expr list)
{
    if (!list)
    {
        return list;
    }

    Expr prev = nil;
    Expr expr = list;
    while (is_cons(expr))
    {
        Expr next = cdr(expr);
        rplacd(expr, prev);
        prev = expr;
        expr = next;
    }
    if (expr)
    {
        Expr iter;
        for (iter = prev; cdr(iter); iter = cdr(iter))
        {
            Expr next = car(iter);
            rplaca(iter, expr);
            expr = next;
        }
        Expr next = car(iter);
        rplaca(iter, expr);
        rplacd(iter, next);
    }
    return prev;
}

Expr append(Expr a, Expr b)
{
    return a ? cons(car(a), append(cdr(a), b)) : b;
}
