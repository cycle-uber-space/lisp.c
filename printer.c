
#include "lisp.h"

static bool is_quote_call(Expr exp)
{
    return is_cons(exp) && eq(car(exp), intern("quote"));
}

void render_expr(Expr exp, Expr out);

void render_cons(Expr exp, Expr out)
{
#if LISP_PRINTER_RENDER_QUOTE
    if (is_quote_call(exp))
    {
        stream_put_char(out, '\'');
        render_expr(cadr(exp), out);
        return;
    }
#endif
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

void render_gensym(Expr exp, Expr out)
{
    LISP_ASSERT_DEBUG(is_gensym(exp));
    U64 const num = expr_data(exp);
    stream_put_string(out, "#:G");
    stream_put_u64(out, num);
}

void render_string(Expr exp, Expr out)
{
    stream_put_char(out, '"');
    char const * str = string_value(exp);
    size_t const len = string_length(exp);
    for (size_t i = 0; i < len; ++i)
    {
        // TODO \u****
        // TODO \U********
        char const ch = str[i];
        switch (ch)
        {
        case '"':
            stream_put_char(out, '\\');
            stream_put_char(out, '"');
            break;
        case '\n':
            stream_put_char(out, '\\');
            stream_put_char(out, 'n');
            break;
        case '\t':
            stream_put_char(out, '\\');
            stream_put_char(out, 't');
            break;
        default:
            if (ch == 0x1b) // TODO use a function to test what to escape
            {
                stream_put_char(out, '\\');
                stream_put_char(out, 'x');
                stream_put_x64(out, ch);
            }
            else
            {
                stream_put_char(out, ch);
            }
            break;
        }
    }
    stream_put_char(out, '"');
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
    case TYPE_GENSYM:
        render_gensym(exp, out);
        break;
    case TYPE_STRING:
        render_string(exp, out);
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
