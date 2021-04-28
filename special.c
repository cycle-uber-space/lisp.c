
#include "common.h"

void special_init(SpecialState * special)
{
    memset(special, 0, sizeof(SpecialState));
}

void special_quit(SpecialState * special)
{
}

bool is_special(Expr exp)
{
    return expr_type(exp) == TYPE_SPECIAL;
}

Expr lisp_make_special(SpecialState * special, char const * name, SpecialFun fun)
{
    LISP_ASSERT(special->num < LISP_MAX_SPECIALS);
    U64 const index = special->num++;
    SpecialInfo * info = special->info + index;
    info->name = name; /* TODO take ownership of name? */
    info->fun = fun;
    return make_expr(TYPE_SPECIAL, index);
}

static SpecialInfo * _special_expr_to_info(SpecialState * special, Expr exp)
{
    LISP_ASSERT(is_special(exp));
    U64 const index = expr_data(exp);
    LISP_ASSERT(index < special->num);
    return special->info + index;
}

char const * lisp_special_name(SpecialState * special, Expr exp)
{
    SpecialInfo * info = _special_expr_to_info(special, exp);
    return info->name;
}

SpecialFun lisp_special_fun(SpecialState * special, Expr exp)
{
    SpecialInfo * info = _special_expr_to_info(special, exp);
    return info->fun;
}

#if LISP_GLOBAL_API

Expr make_special(char const * name, SpecialFun fun)
{
    return lisp_make_special(&global.special, name, fun);
}

#endif
