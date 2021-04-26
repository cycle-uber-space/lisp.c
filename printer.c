
#include "lisp.h"

void render_expr(Expr exp, Expr out);

void render_expr(Expr exp, Expr out)
{
    switch (expr_type(exp))
    {
    case TYPE_NIL:
        LISP_ASSERT_DEBUG(expr_data(exp) == 0);
        stream_put_string(out, "nil");
        break;
    case TYPE_SYMBOL:
        stream_put_string(out, symbol_name(exp));
        break;
    default:
        LISP_FAIL("cannot print expression %016" PRIx64 "\n", exp);
        break;
    }
}
