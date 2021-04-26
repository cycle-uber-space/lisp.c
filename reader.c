
#include "lisp.h"

static bool is_whitespace(char ch)
{
    return
        ch == ' ' ||
        ch == '\n' ||
        ch == '\t';
}

static bool is_symbol_start(char ch)
{
    return
        ch != 0 &&
        !is_whitespace(ch) &&
        ch != '"' &&
        ch != '(' && ch != ')' &&
        ch != ';' && ch != '\'';
}

static bool is_symbol_part(char ch)
{
    return is_symbol_start(ch);
}

static void skip_whitespace_or_comment(Expr in)
{
whitespace:
    while (is_whitespace(stream_peek_char(in)))
    {
        stream_skip_char(in);
    }

    if (stream_peek_char(in) != ';')
    {
        return;
    }

    stream_skip_char(in);

comment:
    if (stream_peek_char(in) == 0)
    {
        return;
    }

    if (stream_peek_char(in) == '\n')
    {
        stream_skip_char(in);
        goto whitespace;
    }

    stream_skip_char(in);
    goto comment;
}

static Expr parse_list(Expr in)
{
    return nil;
}

static Expr parse_expr(Expr in)
{
    skip_whitespace_or_comment(in);

    if (stream_peek_char(in) == '(')
    {
        return parse_list(in);
    }
    else if (is_symbol_start(stream_peek_char(in)))
    {
        char lexeme[4096];
        Expr tok = lisp_make_buffer_output_stream(&global.stream, 4096, lexeme);
        stream_put_char(tok, stream_get_char(in));

    symbol_loop:
        if (is_symbol_part(stream_peek_char(in)))
        {
            stream_put_char(tok, stream_get_char(in));
            goto symbol_loop;
        }
        else
        {
            goto symbol_done;
        }

    symbol_done:
        stream_put_char(tok, 0);
        Expr const ret = intern(lexeme);
        stream_release(tok);
        return ret;
    }
    else
    {
        LISP_FAIL("cannot read expression, unexpected '%c'\n", stream_peek_char(in));
        return nil;
    }
}

Expr read_one_from_string(char const * src)
{
    Expr const in = make_string_input_stream(src);
    Expr ret = parse_expr(in);
    stream_release(in);
    //println(ret);
    return ret;
}
