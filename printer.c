
#include "lisp.h"

void render_expr(Expr exp, Expr out);

void render_cons(Expr exp, Expr out)
{
    stream_put_char(out, '(');
    render_expr(car(exp), out);

    for (Expr tmp = cdr(exp); tmp; tmp = cdr(tmp))
    {
        if (tmp == exp)
        {
            stream_put_string(out, " ...");
            break;
        }
        else if (is_cons(tmp))
        {
            stream_put_char(out, ' ');
            render_expr(car(tmp), out);
        }
        else
        {
            stream_put_string(out, " . ");
            render_expr(tmp, out);
            break;
        }
    }

    stream_put_char(out, ')');
}

void render_special(Expr exp, Expr out)
{
    stream_put_string(out, "#:<special operator");
    char const * name = special_name(exp);
    if (name)
    {
        stream_put_string(out, " ");
        stream_put_string(out, name);
    }
    stream_put_string(out, ">");
}

void render_builtin(Expr exp, Expr out)
{
    stream_put_string(out, "#:<core function");
    char const * name = builtin_name(exp);
    if (name)
    {
        stream_put_string(out, " ");
        stream_put_string(out, name);
    }
    stream_put_string(out, ">");
}

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
    case TYPE_CONS:
        render_cons(exp, out);
        break;
    case TYPE_SPECIAL:
        render_special(exp, out);
        break;
    case TYPE_BUILTIN:
        render_builtin(exp, out);
        break;
    default:
        LISP_FAIL("cannot print expression %016" PRIx64 "\n", exp);
        break;
    }
}
