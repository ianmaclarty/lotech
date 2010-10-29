/* Copyright (C) 2010 Ian MacLarty */
#include "ltcommon.h"

#include <cstdarg>

void ltAbort(const char *fmt, ...) {
    va_list argp;
    fprintf(stderr, "Error: ");
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    fprintf(stderr, "\n");
    exit(1);
}
