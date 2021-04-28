
#include "common.h"

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

Expr eval_body(Expr exps, Expr env)
{
    Expr ret = nil;
    for (Expr tmp = exps; tmp; tmp = cdr(tmp))
    {
        Expr const exp = car(tmp);
        ret = eval(exp, env);
    }
    return ret;
}

/* TODO move these */

/* (lit clo <env> <args> . <body>) */

bool is_closure(Expr exp, Expr kind)
{
    return is_cons(exp) &&
        eq(intern("lit"), car(exp)) &&
        is_cons(cdr(exp)) &&
        eq(kind, cadr(exp));
}

bool is_function(Expr exp)
{
    return is_closure(exp, intern("clo"));
}

bool is_macro(Expr exp)
{
    return is_closure(exp, intern("mac"));
}

Expr closure_env(Expr exp)
{
    return caddr(exp);
}

Expr closure_args(Expr exp)
{
    return cadddr(exp);
}

Expr closure_body(Expr exp)
{
    return cddddr(exp);
}

static void bind_args(Expr env, Expr vars, Expr vals)
{
    env_destructuring_bind(env, vars, vals);
}

static Expr wrap_env(Expr lenv)
{
    return make_env(lenv);
}

static Expr make_call_env_from(Expr lenv, Expr vars, Expr vals)
{
    Expr cenv = wrap_env(lenv);
    bind_args(cenv, vars, vals);
    return cenv;
}

Expr apply(Expr name, Expr args, Expr env)
{
    if (is_builtin(name))
    {
        // TODO parse keyword args
        Expr kwargs = nil;
        Expr vals = eval_list(args, env);
        return builtin_fun(name)(vals, kwargs, env);
    }
    else if (is_special(name))
    {
        // TODO parse keyword args
        Expr kwargs = nil;
        Expr vals = args;
        return special_fun(name)(vals, kwargs, env);
    }
    else if (is_function(name))
    {
        // TODO parse keyword args
        Expr kwargs = nil;
        Expr vals = eval_list(args, env);
        Expr body = closure_body(name);
        return eval_body(body, make_call_env_from(closure_env(name), closure_args(name), vals));
    }
    else if (is_macro(name))
    {
        Expr body = closure_body(name);
        Expr exp = eval_body(body, make_call_env_from(closure_env(name), closure_args(name), args));
        return eval(exp, env);
    }
    else
    {
        // TODO check for unlimited recursion
        return apply(eval(name, env), args, env);
    }
}

Expr eval(Expr exp, Expr env)
{
    if (exp == nil)
    {
        return nil;
    }

    switch (expr_type(exp))
    {
    case TYPE_STRING:
        return exp;
    case TYPE_SYMBOL:
        if (exp == intern("*env*"))
        {
            return env;
        }
        return env_get(env, exp);
    case TYPE_CONS:
        return apply(car(exp), cdr(exp), env);
    default:
        LISP_FAIL("cannot evaluate %s\n", repr(exp));
        return nil;
    }
}
