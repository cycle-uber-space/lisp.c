
#include "lisp.h"

void gensym_init(GensymState * gensym)
{
    memset(gensym, 0, sizeof(GensymState));
}

void gensym_quit(GensymState * gensym)
{
}

bool is_gensym(Expr exp)
{
    return expr_type(exp) == TYPE_GENSYM;
}

Expr lisp_gensym(GensymState * gensym)
{
    return make_expr(TYPE_GENSYM, gensym->counter++);
}
