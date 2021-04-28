
#include "common.h"

void system_init(SystemState * system)
{
    symbol_init(&system->symbol);
    cons_init(&system->cons);
    gensym_init(&system->gensym);
    string_init(&system->string);
    stream_init(&system->stream);
    special_init(&system->special);
    builtin_init(&system->builtin);
}

void system_quit(SystemState * system)
{
    special_quit(&system->special);
    builtin_quit(&system->builtin);
    string_quit(&system->string);
    gensym_quit(&system->gensym);
    stream_quit(&system->stream);
    cons_quit(&system->cons);
    symbol_quit(&system->symbol);
}

void load_file(char const * path, Expr env)
{
    Expr const in = make_file_input_stream_from_path(path);
    Expr exp = nil;
    while (maybe_parse_expr(in, &exp))
    {
        eval(exp, env);
    }
    stream_release(in);
}
