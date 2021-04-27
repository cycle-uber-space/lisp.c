
#include "lisp.h"

#define LISP_SYM_QUOTE intern("quote")
#define LISP_SYM_IF intern("if")
#define LISP_SYM_BACKQUOTE intern("backquote")
#define LISP_SYM_UNQUOTE intern("unquote")
#define LISP_SYM_UNQUOTE_SPLICING intern("unquote-splicing")

bool is_op(Expr exp, Expr name)
{
    return is_cons(exp) && car(exp) == name;
}

bool is_quote(Expr exp)
{
    return is_op(exp, LISP_SYM_QUOTE);
}

bool is_if(Expr exp)
{
    return is_op(exp, LISP_SYM_IF);
}

bool is_unquote(Expr exp)
{
    return is_op(exp, LISP_SYM_UNQUOTE);
}

bool is_unquote_splicing(Expr exp)
{
    return is_op(exp, LISP_SYM_UNQUOTE_SPLICING);
}

Expr eval(Expr exp, Expr env);

Expr eval_list(Expr exps, Expr env)
{
    Expr ret = nil;
    for (Expr tmp = exps; tmp; tmp = cdr(tmp))
    {
        Expr const exp = car(tmp);
        ret = cons(eval(exp, env), ret);
    }
    return nreverse(ret);
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
    else if (is_special(func))
    {
        // TODO parse keyword args
        Expr kwargs = nil;
        Expr vals = args;
        return special_fun(func)(vals, kwargs, env);
    }
    else
    {
        LISP_FAIL("cannot apply %s to %s\n", repr(name), repr(args));
        return nil;
    }
}

static Expr backquote(Expr exp, Expr env);

static Expr backquote_list(Expr seq, Expr env)
{
    if (seq)
    {
        Expr item = car(seq);
        Expr rest = cdr(seq);
        if (is_unquote_splicing(item))
        {
            return append(eval(cadr(item), env), backquote_list(rest, env));
        }
        else
        {
            return cons(backquote(item, env), backquote_list(rest, env));
        }
    }
    else
    {
        return nil;
    }
}

static Expr backquote(Expr exp, Expr env)
{
    if (is_cons(exp))
    {
        if (is_unquote(exp))
        {
            return eval(cadr(exp), env);
        }
        else
        {
            return backquote_list(exp, env);
        }
    }
    else
    {
        return exp;
    }
}

static Expr eval_backquote(Expr exp, Expr env)
{
    return backquote(cadr(exp), env);
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
#if LISP_EVAL_QUOTE
        if (car(exp) == intern("quote"))
        {
            return cadr(exp);
        }
        else
#endif
#if LISP_EVAL_IF
        if (car(exp) == intern("if"))
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
#endif
        if (car(exp) == LISP_SYM_BACKQUOTE)
        {
            return eval_backquote(exp, env);
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
