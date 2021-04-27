
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

static Expr parse_expr(SystemState * sys, Expr in);

static Expr parse_list(SystemState * sys, Expr in)
{
    Expr exp = nil;
    Expr head = nil;
    Expr tail = nil;

    if (stream_peek_char(in) != '(')
    {
        LISP_FAIL("expected '(', got '%c'\n", stream_peek_char(in));
        return nil;
    }

    stream_skip_char(in);

list_loop:
    skip_whitespace_or_comment(in);

    if (stream_peek_char(in) == 0)
    {
        LISP_FAIL("unexpected eof\n");
        return nil;
    }

    if (stream_peek_char(in) == ')')
    {
        goto list_done;
    }

    exp = parse_expr(sys, in);

    // TODO get rid of artifical symbol dependence for dotted lists
    if (exp == intern("."))
    {
        exp = parse_expr(sys, in);
        lisp_rplacd(&sys->cons, tail, exp);

        skip_whitespace_or_comment(in);

        goto list_done;
    }
    else
    {
        Expr next = lisp_cons(&sys->cons, exp, nil);
        if (head)
        {
            lisp_rplacd(&sys->cons, tail, next);
            tail = next;
        }
        else
        {
            head = tail = next;
        }
    }

    goto list_loop;

list_done:
    // TODO use expect_char(in, ')')
    if (stream_peek_char(in) != ')')
    {
        LISP_FAIL("missing ')'\n");
        return nil;
    }
    stream_skip_char(in);

    return head;
}

static Expr parse_expr(SystemState * sys, Expr in)
{
    skip_whitespace_or_comment(in);

    if (stream_peek_char(in) == '(')
    {
        return parse_list(sys, in);
    }
#if LISP_READER_PARSE_QUOTE
    else if (stream_peek_char(in) == '\'')
    {
        stream_skip_char(in);
        Expr const exp = list_2(intern("quote"), parse_expr(sys, in));
        return exp;
    }
#endif
    else if (is_symbol_start(stream_peek_char(in)))
    {
        char lexeme[4096];
        Expr tok = lisp_make_buffer_output_stream(&sys->stream, 4096, lexeme);
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

bool lisp_maybe_parse_expr(SystemState * sys, Expr in, Expr * exp)
{
    skip_whitespace_or_comment(in);
    if (lisp_stream_at_end(&sys->stream, in))
    {
        return false;
    }
    *exp = parse_expr(sys, in);
    return true;
}

Expr lisp_read_one_from_string(SystemState * sys, char const * src)
{
    Expr const in = lisp_make_string_input_stream(&sys->stream, src);
    Expr ret = parse_expr(sys, in);
    lisp_stream_release(&sys->stream, in);
    //println(ret);
    return ret;
}

bool maybe_parse_expr(Expr in, Expr * exp)
{
    return lisp_maybe_parse_expr(&global, in, exp);
}

Expr read_one_from_string(char const * src)
{
    return lisp_read_one_from_string(&global, src);
}
