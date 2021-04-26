
#include "lisp.h"

Expr eval(Expr exp, Expr env)
{
    //fprintf(stderr, "EVAL %016" PRIx64 "\n", exp);
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
    default:
        LISP_FAIL("cannot evaluate %s\n", repr(exp));
        return nil;
    }
}
