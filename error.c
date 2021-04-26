
#include "lisp.h"

void error_fail(char const * fmt, ...)
{
    FILE * const file = stderr;
    va_list ap;
    va_start(ap, fmt);
    fprintf(file, LISP_RED "[FAIL] " LISP_RESET);
    vfprintf(file, fmt, ap);
    va_end(ap);
    exit(1);
}

void error_warn(char const * fmt, ...)
{
    FILE * const file = stderr;
    va_list ap;
    va_start(ap, fmt);
    fprintf(file, LISP_YELLOW "[WARN] " LISP_RESET);
    vfprintf(file, fmt, ap);
    va_end(ap);
}
