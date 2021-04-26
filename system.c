
#include "lisp.h"

void system_init(SystemState * system)
{
    symbol_init(&system->symbol);
    cons_init(&system->cons);
    stream_init(&system->stream);
}

void system_quit(SystemState * system)
{
    stream_quit(&system->stream);
    cons_quit(&system->cons);
    symbol_quit(&system->symbol);
}
