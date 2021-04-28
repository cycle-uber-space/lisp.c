
#include "common.h"

static Expr env_vars(Expr env);
static void env_set_vars(Expr env, Expr vars);
static Expr env_vals(Expr env);
static void env_set_vals(Expr env, Expr vals);
static Expr env_outer(Expr env);

static Expr _env_find_local(Expr env, Expr var)
{
    Expr vars = env_vars(env);
    Expr vals = env_vals(env);
    while (vars)
    {
        if (car(vars) == var)
        {
            return vals;
        }
        vars = cdr(vars);
        vals = cdr(vals);
    }
    return nil;
}

static Expr _env_find_global(Expr env, Expr var)
{
    while (env)
    {
        Expr const vals = _env_find_local(env, var);
        if (vals)
        {
            return vals;
        }
        else
        {
            env = env_outer(env);
        }
    }
    return nil;
}

Expr make_env(Expr outer)
{
    // ((<vars> . <vals>) . <outer>)
    // TODO add dummy conses as sentinels for vars and vals
    return cons(cons(nil, nil), outer);
}

static Expr env_vars(Expr env)
{
    return caar(env);
}

static void env_set_vars(Expr env, Expr vars)
{
    rplaca(car(env), vars);
}

static Expr env_vals(Expr env)
{
    return cdar(env);
}

static void env_set_vals(Expr env, Expr vals)
{
    rplacd(car(env), vals);
}

static Expr env_outer(Expr env)
{
    return cdr(env);
}

void env_def(Expr env, Expr var, Expr val)
{
    Expr const vals = _env_find_local(env, var);
    if (vals)
    {
        rplaca(vals, val);
    }
    else
    {
        env_set_vars(env, cons(var, env_vars(env)));
        env_set_vals(env, cons(val, env_vals(env)));
    }
}

void env_del(Expr env, Expr var)
{
    Expr prev_vars = nil;
    Expr prev_vals = nil;

    Expr vars = env_vars(env);
    Expr vals = env_vals(env);
    while (vars)
    {
        if (car(vars) == var)
        {
            if (prev_vars)
            {
                LISP_ASSERT(prev_vals);
                rplacd(prev_vars, cdr(vars));
                rplacd(prev_vals, cdr(vals));
            }
            else
            {
                env_set_vars(env, cdr(vars));
                env_set_vals(env, cdr(vals));
            }
            return;
        }

        prev_vars = vars;
        prev_vals = vals;
        vars = cdr(vars);
        vals = cdr(vals);
    }

    LISP_FAIL("unbound variable %s\n", repr(var));
}

bool env_can_set(Expr env, Expr var)
{
    Expr const tmp = _env_find_global(env, var);
    return tmp != nil;
}

Expr env_get(Expr env, Expr var)
{
    Expr const vals = _env_find_global(env, var);
    if (vals)
    {
        return car(vals);
    }
    else
    {
        LISP_FAIL("unbound variable %s\n", repr(var));
        return nil;
    }
}

void env_set(Expr env, Expr var, Expr val)
{
    Expr const vals = _env_find_global(env, var);
    if (vals)
    {
        rplaca(vals, val);
    }
    else
    {
        LISP_FAIL("unbound variable %s\n", repr(var));
    }
}

void env_destructuring_bind(Expr env, Expr vars, Expr vals)
{
    if (vars == nil)
    {
        if (vals != nil)
        {
            LISP_FAIL("no more parameters to bind\n");
        }
    }
    else if (is_cons(vars))
    {
        while (vars)
        {
            if (is_cons(vars))
            {
                LISP_ASSERT(is_cons(vals));
                env_destructuring_bind(env, car(vars), car(vals));
                vars = cdr(vars);
                vals = cdr(vals);
            }
            else
            {
                env_destructuring_bind(env, vars, vals);
                break;
            }
        }
    }
    else
    {
        env_def(env, vars, vals);
    }
}
