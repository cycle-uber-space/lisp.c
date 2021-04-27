
#include "lisp.h"

static Expr string_alloc(StringState * string, size_t len)
{
    if (string->count < LISP_MAX_STRINGS)
    {
        U64 const index = string->count;
        string->values[index] = (char *) LISP_MALLOC(len + 1);
        ++string->count;
        return make_expr(TYPE_STRING, index);
    }

    LISP_FAIL("cannot make string of length %d\n", (int) len);
    return nil;
}

static char * string_buffer(StringState * string, Expr exp)
{
    LISP_ASSERT(is_string(exp));

    U64 const index = expr_data(exp);
    if (index >= string->count)
    {
        LISP_FAIL("illegal string index %" PRIu64 "\n", index);
    }

    return string->values[index];
}

void string_init(StringState * string)
{
    memset(string, 0, sizeof(StringState));
    string->values = (char **) LISP_MALLOC(sizeof(char *) * LISP_MAX_STRINGS);
}

void string_quit(StringState * string)
{
    LISP_FREE(string->values);
    memset(string, 0, sizeof(StringState));
}

bool is_string(Expr exp)
{
    return expr_type(exp) == TYPE_STRING;
}

Expr lisp_make_string(StringState * string, char const * str)
{
    size_t const len = strlen(str);

    /* TODO fixstrs sound attractive,
       but the string_value API breaks
       b/c it has to return a pointer */

    Expr ret = string_alloc(string, len);
    memcpy(string_buffer(string, ret), str, len + 1);
    return ret;
}

char const * lisp_string_value(StringState * string, Expr exp)
{
    return string_buffer(string, exp);
}

U64 lisp_string_length(StringState * string, Expr exp)
{
    // TODO cache this
    return (U64) strlen(lisp_string_value(string, exp));
}

Expr make_string(char const * str)
{
    return lisp_make_string(&global.string, str);
}

char const * string_value(Expr exp)
{
    return lisp_string_value(&global.string, exp);
}

U64 string_length(Expr exp)
{
    return lisp_string_length(&global.string, exp);
}
