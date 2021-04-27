
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

static char parse_hex_digit(Expr in, char val)
{
    char const ch = stream_get_char(in);

    if (ch >= '0' && ch <= '9')
    {
        val *= 16;
        val += ch - '0';
    }

    else if (ch >= 'a' && ch <= 'f')
    {
        val *= 16;
        val += 10 + ch - 'a';
    }

    else if (ch >= 'A' && ch <= 'F')
    {
        val *= 16;
        val += 10 + ch - 'A';
    }

    else
    {
        LISP_FAIL("malformed string");
    }

    return val;
}

static Expr parse_string(SystemState * sys, Expr in)
{
    enum
    {
        STATE_DEFAULT,
        STATE_ESCAPE,
    } state = STATE_DEFAULT;

    if (stream_peek_char(in) != '"')
    {
        LISP_FAIL("missing '\"'\n");
        return nil;
    }
    stream_skip_char(in);

    char lexeme[4096];
    Expr tok = lisp_make_buffer_output_stream(&sys->stream, 4096, lexeme);

string_loop:
    if (stream_peek_char(in) == 0)
    {
        LISP_FAIL("unexpected eof in string\n");
        return nil;
    }

    else if (state == STATE_DEFAULT)
    {
        if (stream_peek_char(in) == '"')
        {
            stream_skip_char(in);
            goto string_done;
        }
        else if (stream_peek_char(in) == '\\')
        {
            stream_skip_char(in);
            state = STATE_ESCAPE;
        }
        else
        {
            stream_put_char(tok, stream_get_char(in));
        }
    }

    else if (state == STATE_ESCAPE)
    {
        if (stream_peek_char(in) == 'n')
        {
            stream_skip_char(in);
            stream_put_char(tok, '\n');
        }

        else if (stream_peek_char(in) == 't')
        {
            stream_skip_char(in);
            stream_put_char(tok, '\t');
        }

        else if (stream_peek_char(in) == 'x')
        {
            stream_skip_char(in);

            char val = 0;
            val = parse_hex_digit(in, val);
            val = parse_hex_digit(in, val);

            /* TODO check for more digits? */

            stream_put_char(tok, val);
        }

        else
        {
            stream_put_char(tok, stream_get_char(in));
        }

        state = STATE_DEFAULT;
    }

    else
    {
        LISP_FAIL("internal error\n");
        return nil;
    }

    goto string_loop;

string_done:
    stream_put_char(tok, 0);
    Expr const ret = make_string(lexeme);
    stream_release(tok);
    return ret;
}

static Expr parse_expr(SystemState * sys, Expr in)
{
    skip_whitespace_or_comment(in);

    if (stream_peek_char(in) == '(')
    {
        return parse_list(sys, in);
    }
    else if (stream_peek_char(in) == '"')
    {
        return parse_string(sys, in);
    }
#if LISP_READER_PARSE_QUOTE
    else if (stream_peek_char(in) == '\'')
    {
        stream_skip_char(in);
        Expr const exp = list_2(intern("quote"), parse_expr(sys, in));
        return exp;
    }
    else if (stream_peek_char(in) == '`')
    {
        stream_skip_char(in);
        Expr const exp = list_2(intern("backquote"), parse_expr(sys, in));
        return exp;
    }
    else if (stream_peek_char(in) == ',')
    {
        stream_skip_char(in);
        if (stream_peek_char(in) == '@')
        {
            stream_skip_char(in);
            return list_2(intern("unquote-splicing"), parse_expr(sys, in));
        }
        return list_2(intern("unquote"), parse_expr(sys, in));
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
