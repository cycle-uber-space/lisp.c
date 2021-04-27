
#include "lisp.h"

Expr f_eq(Expr args, Expr kwargs, Expr env)
{
    if (is_nil(args))
    {
        LISP_FAIL("not enough arguments in call %s\n", repr(cons(intern("eq"), args)));
    }
    Expr prv = car(args);
    Expr tmp = cdr(args);
    if (is_nil(tmp))
    {
        LISP_FAIL("not enough arguments in call %s\n", repr(cons(intern("eq"), args)));
    }
    for (; tmp; tmp = cdr(tmp))
    {
        Expr const exp = car(tmp);
        if (prv != exp)
        {
            return nil;
        }
        prv = exp;
    }
    return intern("t");
}

Expr f_equal(Expr args, Expr kwargs, Expr env)
{
    if (is_nil(args))
    {
        LISP_FAIL("not enough arguments in call %s\n", repr(cons(intern("equal"), args)));
    }
    Expr prv = car(args);
    Expr tmp = cdr(args);
    if (is_nil(tmp))
    {
        LISP_FAIL("not enough arguments in call %s\n", repr(cons(intern("equal"), args)));
    }
    for (; tmp; tmp = cdr(tmp))
    {
        Expr const exp = car(tmp);
        if (!equal(prv, exp))
        {
            return nil;
        }
        prv = exp;
    }
    return intern("t");
}

Expr f_cons(Expr args, Expr kwargs, Expr env)
{
    LISP_ASSERT(args != nil);
    LISP_ASSERT(cdr(args) != nil);
    LISP_ASSERT(cddr(args) == nil);

    Expr const exp1 = car(args);
    Expr const exp2 = cadr(args);
    return cons(exp1, exp2);
}

Expr f_car(Expr args, Expr kwargs, Expr env)
{
    LISP_ASSERT(args != nil);
    LISP_ASSERT(cdr(args) == nil);

    Expr const exp1 = car(args);
    return car(exp1);
}

Expr f_cdr(Expr args, Expr kwargs, Expr env)
{
    LISP_ASSERT(args != nil);
    LISP_ASSERT(cdr(args) == nil);

    Expr const exp1 = car(args);
    return cdr(exp1);
}

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
    env_defun(env, "eq", f_eq);
    env_defun(env, "equal", f_eq);
    env_defun(env, "cons", f_cons);
    env_defun(env, "car", f_car);
    env_defun(env, "cdr", f_cdr);
    env_defun(env, "println", f_println);
    return env;
}
