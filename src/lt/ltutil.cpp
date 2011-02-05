#include <sys/stat.h> 
#include <stdio.h>
#include <stdlib.h>
#include <cstdarg>

#include "ltutil.h"

void ltAbort(const char *fmt, ...) {
    va_list argp;
    fprintf(stderr, "Error: ");
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    fprintf(stderr, "\n");
    exit(1);
}

void ltLog(const char *fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    fprintf(stderr, "\n");
}

bool ltFileExists(const char *file) {
    struct stat info;
    return stat(file, &info) == 0;
}
