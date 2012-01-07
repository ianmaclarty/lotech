#include <glob.h>
#include <sys/stat.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdarg>

#include "ltutil.h"
#include "ltprotocol.h"

void ltAbort() {
    ltLog("FATAL INTERNAL ERROR. ABORTING.");
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

char* ltGlob(const char **patterns) {
    glob_t globbuf;
    globbuf.gl_offs = 0;
    int flags = 0;
    while (*patterns != NULL) {
        glob(*patterns, flags, NULL, &globbuf);
        flags |= GLOB_APPEND;
        patterns++;
    }
    unsigned int size = 0;
    for (unsigned int i = 0; i < globbuf.gl_pathc; i++) {
        size += strlen(globbuf.gl_pathv[i]) + 1;
    }
    size++;
    char* matches = new char[size];
    char* ptr = matches;
    for (unsigned int i = 0; i < globbuf.gl_pathc; i++) {
        strcpy(ptr, globbuf.gl_pathv[i]);
        ptr += strlen(globbuf.gl_pathv[i]) + 1;
    }
    *ptr = '\0';
    globfree(&globbuf);
    return matches;
}
