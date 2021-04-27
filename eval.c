
#include "lisp.h"

Expr eval(Expr exp, Expr env);

Expr eval_list(Expr exps, Expr env)
{
    Expr ret = nil;
    for (Expr tmp = exps; tmp; tmp = cdr(tmp))
    {
        Expr const exp = car(tmp);
        ret = cons(eval(exp, env), ret);
    }
    return ret;
}

Expr apply(Expr name, Expr args, Expr env)
{
    Expr func = eval(name, env);
    if (is_builtin(func))
    {
        // TODO parse keyword args
        Expr kwargs = nil;
        Expr vals = eval_list(args, env);
        return builtin_fun(func)(vals, kwargs, env);
    }
    else
    {
        LISP_FAIL("cannot apply %s to %s\n", repr(name), repr(args));
        return nil;
    }
}

Expr eval(Expr exp, Expr env)
{
dispatch:
    //fprintf(stderr, "EVAL %016" PRIx64 "\n", exp);
    //fprintf(stderr, "EVAL %s\n", repr(exp));

    if (exp == nil)
    {
        return nil;
    }

    switch (expr_type(exp))
    {
    case TYPE_SYMBOL:
        if (exp == intern("*env*"))
        {
            return env;
        }
        return env_get(env, exp);
    case TYPE_CONS:
        if (car(exp) == intern("quote"))
        {
            return cadr(exp);
        }
        else if (car(exp) == intern("if"))
        {
            Expr const test = cadr(exp);
            Expr const then = caddr(exp);
            Expr const rest = cdddr(exp) ? cadddr(exp) : nil;
            if (eval(test, env) != nil)
            {
                exp = then;
                goto dispatch;
            }
            else
            {
                exp = rest;
                goto dispatch;
            }
        }
        else
        {
            return apply(car(exp), cdr(exp), env);
        }
    default:
        LISP_FAIL("cannot evaluate %s\n", repr(exp));
        return nil;
    }
}
