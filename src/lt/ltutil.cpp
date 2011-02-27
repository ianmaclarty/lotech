#include <sys/stat.h> 
#include <stdio.h>
#include <stdlib.h>
#include <cstdarg>

#include "ltutil.h"
#include "ltprotocol.h"

void ltAbort(const char *fmt, ...) {
    va_list argp;
    fprintf(stderr, "Error: ");
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    fprintf(stderr, "\n");
    exit(1);
}

#define MAX_MSG_LEN 2048

void ltLog(const char *fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    fprintf(stderr, "\n");
    if (ltAmClient()) {
        char msg[MAX_MSG_LEN];
        va_list argp;
        va_start(argp, fmt);
        vsnprintf(msg, MAX_MSG_LEN, fmt, argp);
        va_end(argp);
        ltClientLog(msg);
    }    
}

bool ltFileExists(const char *file) {
    struct stat info;
    return stat(file, &info) == 0;
}
