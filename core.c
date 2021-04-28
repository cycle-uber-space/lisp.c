
#include "common.h"

static void env_defun(Expr env, char const * name, BuiltinFun fun)
{
    env_def(env, intern(name), make_builtin(name, fun));
}

static void env_defspecial(Expr env, char const * name, SpecialFun fun)
{
    env_def(env, intern(name), make_special(name, fun));
}

Expr s_quote(Expr args, Expr kwargs, Expr env)
{
    return car(args);
}

Expr s_def(Expr args, Expr kwargs, Expr env)
{
    // TODO look for env in kwargs
    env_def(env, car(args), eval(cadr(args), env));
    return nil;
}

Expr s_if(Expr args, Expr kwargs, Expr env)
{
    if (eval(car(args), env) != nil)
    {
        return eval(cadr(args), env);
    }
    else if (cddr(args))
    {
        return eval(caddr(args), env);
    }
    else
    {
        return nil;
    }
}

Expr s_lambda(Expr args, Expr kwargs, Expr env)
{
    Expr const fun_args = car(args);
    Expr const fun_body = cdr(args);
    return cons(intern("lit"), cons(intern("clo"), cons(env, cons(fun_args, fun_body))));
}

bool is_unquote(Expr exp)
{
    return is_named_call(exp, LISP_SYM_UNQUOTE);
}

bool is_unquote_splicing(Expr exp)
{
    return is_named_call(exp, LISP_SYM_UNQUOTE_SPLICING);
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

Expr s_backquote(Expr args, Expr kwargs, Expr env)
{
    return backquote(car(args), env);
}

Expr s_syntax(Expr args, Expr kwargs, Expr env)
{
    Expr const fun_args = car(args);
    Expr const fun_body = cdr(args);
    return cons(intern("lit"), cons(intern("mac"), cons(env, cons(fun_args, fun_body))));
}

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

    return LISP_SYMBOL_T;
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

Expr f_gensym(Expr args, Expr kwargs, Expr env)
{
    return lisp_gensym(&global.gensym);
}

Expr f_load_file(Expr args, Expr kwargs, Expr env)
{
    load_file(string_value(first(args)), env);
    return nil;
}

Expr make_core_env()
{
    Expr env = make_env(nil);

    env_def(env, intern("t"), intern("t"));

    env_defspecial(env, "quote", s_quote);
    env_defspecial(env, "if", s_if);
    env_defspecial(env, "def", s_def);
    env_defspecial(env, "lambda", s_lambda);
    env_defspecial(env, "syntax", s_syntax);
    env_defspecial(env, "backquote", s_backquote);

    env_defun(env, "eq", f_eq);
    env_defun(env, "equal", f_equal);
    env_defun(env, "cons", f_cons);
    env_defun(env, "car", f_car);
    env_defun(env, "cdr", f_cdr);
    env_defun(env, "println", f_println);

    env_defun(env, "gensym", f_gensym);
    env_defun(env, "load-file", f_load_file);

    return env;
}
