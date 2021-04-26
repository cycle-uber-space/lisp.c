
#include "lisp.h"

static void _cons_realloc(ConsState * cons)
{
    cons->pairs = (struct Pair *) LISP_REALLOC(cons->pairs, sizeof(struct Pair) * cons->max);
    if (!cons->pairs)
    {
        LISP_FAIL("cons memory allocation failed\n");
    }
}

static void _cons_maybe_realloc(ConsState * cons)
{
    if (cons->num < cons->max)
    {
        return;
    }

    if (LISP_MAX_CONSES == -1 || cons->max * 2 <= (U64) LISP_MAX_CONSES)
    {
        if (cons->max == 0)
        {
            cons->max = LISP_DEF_CONSES;
        }
        else
        {
            cons->max *= 2;
        }

        _cons_realloc(cons);
        return;
    }

    LISP_FAIL("cons ran over memory budget\n");
}

static struct Pair * _cons_lookup(ConsState * cons, U64 index)
{
    LISP_ASSERT_DEBUG(index < cons->num);
    return &cons->pairs[index];
}

void cons_init(ConsState * cons)
{
    memset(cons, 0, sizeof(ConsState));
}

void cons_quit(ConsState * cons)
{
}

bool is_cons(Expr exp)
{
    return expr_type(exp) == TYPE_CONS;
}

Expr lisp_cons(ConsState * cons, Expr a, Expr b)
{
    _cons_maybe_realloc(cons);

    U64 const index = cons->num++;
    struct Pair * pair = _cons_lookup(cons, index);
    pair->a = a;
    pair->b = b;
    return make_expr(TYPE_CONS, index);
}

Expr lisp_car(ConsState * cons, Expr exp)
{
    LISP_ASSERT(is_cons(exp));

    U64 const index = expr_data(exp);
    struct Pair * pair = _cons_lookup(cons, index);
    return pair->a;
}

Expr lisp_cdr(ConsState * cons, Expr exp)
{
    LISP_ASSERT(is_cons(exp));

    U64 const index = expr_data(exp);
    struct Pair * pair = _cons_lookup(cons, index);
    return pair->b;
}

void lisp_rplaca(ConsState * cons, Expr exp, Expr val)
{
    LISP_ASSERT(is_cons(exp));

    U64 const index = expr_data(exp);
    struct Pair * pair = _cons_lookup(cons, index);
    pair->a = val;
}

void lisp_rplacd(ConsState * cons, Expr exp, Expr val)
{
    LISP_ASSERT(is_cons(exp));

    U64 const index = expr_data(exp);
    struct Pair * pair = _cons_lookup(cons, index);
    pair->b = val;
}

Expr cons(Expr a, Expr b)
{
    return lisp_cons(&global.cons, a, b);
}

Expr car(Expr exp)
{
    return lisp_car(&global.cons, exp);
}

Expr cdr(Expr exp)
{
    return lisp_cdr(&global.cons, exp);
}

void rplaca(Expr exp, Expr val)
{
    lisp_rplaca(&global.cons, exp, val);
}

void rplacd(Expr exp, Expr val)
{
    lisp_rplacd(&global.cons, exp, val);
}
