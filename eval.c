
#include "lisp.h"

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
    default:
        LISP_FAIL("cannot evaluate %s\n", repr(exp));
        return nil;
    }
}
