
#include "lisp.h"

Expr f_println(Expr args, Expr kwargs, Expr env)
{
    Expr out = global.stream.stdout;
    for (Expr tmp = args; tmp; tmp = cdr(tmp))
    {
        if (tmp != args)
        {
            stream_put_char(out, ' ');
        }
        Expr exp = car(tmp);
        render_expr(exp, out);
    }
    stream_put_char(out, '\n');
    return nil;
}

static void env_defun(Expr env, char const * name, BuiltinFun fun)
{
    env_def(env, intern(name), make_builtin(name, fun));
}

Expr make_core_env()
{
    Expr env = make_env(nil);
    env_def(env, intern("t"), intern("t"));
    env_defun(env, "println", f_println);
    return env;
}
