
#include "lisp.h"

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
