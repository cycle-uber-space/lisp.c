
#include "lisp.h"

static Expr parse_expr(Expr in)
{
    return nil;
}

Expr read_one_from_string(char const * src)
{
    Expr const in = make_string_input_stream(src);
    Expr ret = parse_expr(in);
    stream_release(in);
    //println(ret);
    return ret;
}
