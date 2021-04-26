
#include "lisp.h"

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

static Expr _make_buffer_stream(StreamState * stream, size_t size, char * buffer)
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

Expr lisp_make_string_input_stream(StreamState * stream, char const * str)
{
    // TODO copy string into buffer?
    size_t const len = strlen(str);
    return _make_buffer_stream(stream, len + 1, (char *) str);
}

Expr lisp_make_buffer_output_stream(StreamState * stream, size_t size, char * buffer)
{
    return _make_buffer_stream(stream, size, buffer);
}

char lisp_stream_peek_char(StreamState * stream, Expr exp)
{
    U64 const index = expr_data(exp);
    LISP_ASSERT(index < stream->num);
    StreamInfo * info = stream->info + index;
    if (info->file)
    {
        int ch = fgetc(info->file);
        if (ch == EOF)
        {
            return 0;
        }
        ungetc(ch, info->file);
        return ch;
    }

    if (info->buffer)
    {
        if (info->cursor < info->size)
        {
            return info->buffer[info->cursor];
        }
        else
        {
            return 0;
        }
    }

    LISP_FAIL("cannot read from stream\n");
    return 0;
}

void lisp_stream_skip_char(StreamState * stream, Expr exp)
{
    U64 const index = expr_data(exp);
    LISP_ASSERT(index < stream->num);
    StreamInfo * info = stream->info + index;
    if (info->file)
    {
        fgetc(info->file);
        return;
    }

    if (info->buffer)
    {
        ++info->cursor;
        return;
    }

    LISP_FAIL("cannot read from stream\n");
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

void lisp_stream_put_char(StreamState * stream, Expr exp, char ch)
{
    // TODO make this more efficient
    char const tmp[2] = { ch, 0 };
    lisp_stream_put_string(stream, exp, tmp);
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

Expr make_string_input_stream(char const * str)
{
    return lisp_make_string_input_stream(&global.stream, str);
}

char stream_peek_char(Expr exp)
{
    return lisp_stream_peek_char(&global.stream, exp);
}

void stream_skip_char(Expr exp)
{
    lisp_stream_skip_char(&global.stream, exp);
}

void stream_put_char(Expr exp, char ch)
{
    lisp_stream_put_char(&global.stream, exp, ch);
}

void stream_put_string(Expr exp, char const * str)
{
    lisp_stream_put_string(&global.stream, exp, str);
}

void stream_release(Expr exp)
{
    lisp_stream_release(&global.stream, exp);
}
