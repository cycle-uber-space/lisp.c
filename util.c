
#include "lisp.h"

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
