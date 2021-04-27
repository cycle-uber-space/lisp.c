
#include "lisp.h"

void builtin_init(BuiltinState * builtin)
{
    memset(builtin, 0, sizeof(BuiltinState));
}

void builtin_quit(BuiltinState * builtin)
{
}

bool is_builtin(Expr exp)
{
    return expr_type(exp) == TYPE_BUILTIN;
}

Expr lisp_make_builtin(BuiltinState * builtin, char const * name, BuiltinFun fun)
{
    LISP_ASSERT(builtin->num < LISP_MAX_BUILTINS);
    U64 const index = builtin->num++;
    BuiltinInfo * info = builtin->info + index;
    info->name = name; /* TODO take ownership of name? */
    info->fun = fun;
    return make_expr(TYPE_BUILTIN, index);
}

static BuiltinInfo * _builtin_expr_to_info(BuiltinState * builtin, Expr exp)
{
    LISP_ASSERT(is_builtin(exp));
    U64 const index = expr_data(exp);
    LISP_ASSERT(index < builtin->num);
    return builtin->info + index;
}

char const * lisp_builtin_name(BuiltinState * builtin, Expr exp)
{
    BuiltinInfo * info = _builtin_expr_to_info(builtin, exp);
    return info->name;
}

BuiltinFun lisp_builtin_fun(BuiltinState * builtin, Expr exp)
{
    BuiltinInfo * info = _builtin_expr_to_info(builtin, exp);
    return info->fun;
}

Expr make_builtin(char const * name, BuiltinFun fun)
{
    return lisp_make_builtin(&global.builtin, name, fun);
}
