
#include "lisp.h"

static void _symbol_maybe_realloc(SymbolState * symbol)
{
    if (symbol->num < symbol->max)
    {
        return;
    }

    if (LISP_MAX_SYMBOLS == -1 || symbol->max * 2 <= LISP_MAX_SYMBOLS)
    {
        if (symbol->max == 0)
        {
            symbol->max = LISP_DEF_SYMBOLS;
        }
        else
        {
            symbol->max *= 2;
        }

        symbol->names = (char **) LISP_REALLOC(symbol->names, sizeof(char *) * symbol->max);
        if (!symbol->names)
        {
            LISP_FAIL("symbol memory allocation failed\n");
        }
        return;
    }

    LISP_FAIL("intern ran over memory budget\n");
}

void symbol_init(SymbolState * symbol)
{
    LISP_ASSERT_DEBUG(symbol);

    memset(symbol, 0, sizeof(SymbolState));
    _symbol_maybe_realloc(symbol);
}

void symbol_quit(SymbolState * symbol)
{
    for (U64 i = 0; i < symbol->num; i++)
    {
        LISP_FREE(symbol->names[i]);
    }
    LISP_FREE(symbol->names);
    memset(symbol, 0, sizeof(SymbolState));
}

Expr lisp_make_symbol(SymbolState * symbol, char const * name)
{
    LISP_ASSERT(name);
    size_t const len = strlen(name);

    for (U64 index = 0; index < symbol->num; ++index)
    {
        char const * str = symbol->names[index];
        size_t const tmp = strlen(str); // TODO cache this?
        if (len == tmp && !strncmp(name, str, len))
        {
            return make_expr(TYPE_SYMBOL, index);
        }
    }

    _symbol_maybe_realloc(symbol);

    U64 const index = symbol->num;
    char * buffer = (char *) LISP_MALLOC(len + 1);
    memcpy(buffer, name, len);
    buffer[len] = 0;
    symbol->names[index] = buffer;
    ++symbol->num;

    return make_expr(TYPE_SYMBOL, index);
}

char const * lisp_symbol_name(SymbolState * symbol, Expr exp)
{
    LISP_ASSERT(is_symbol(exp));
    U64 const index = expr_data(exp);
    if (index >= symbol->num)
    {
        LISP_FAIL("illegal symbol index %" PRIu64 "\n", index);
    }
    return symbol->names[index];
}

#if LISP_GLOBAL_API
Expr make_symbol(char const * name)
{
    return lisp_make_symbol(&global.symbol, name);
}

char const * symbol_name(Expr exp)
{
#if LISP_SYMBOL_NAME_OF_NIL
    if (exp == nil)
    {
        return "nil";
    }
#endif
    return lisp_symbol_name(&global.symbol, exp);
}

#endif
