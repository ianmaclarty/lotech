#ifndef LTANDROID
#include <glob.h>
#endif
#ifdef LTANDROID
#include <android/log.h>
#endif
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
    char msg[MAX_MSG_LEN];
    va_list argp;
    va_start(argp, fmt);
    vsnprintf(msg, MAX_MSG_LEN, fmt, argp);
    va_end(argp);
#ifdef LTANDROID
    __android_log_print(ANDROID_LOG_INFO, "Lotech", "%s", msg);
#else
    fprintf(stderr, "%s", msg);
    fprintf(stderr, "\n");
#endif
#ifdef LTDEVMODE
    if (ltAmClient()) {
        ltClientLog(msg);
    }    
#endif
}

bool ltFileExists(const char *file) {
    struct stat info;
    return stat(file, &info) == 0;
}

char* ltGlob(const char **patterns) {
#ifndef LTANDROID
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
#else
    ltLog("glob not supported on android");
    return NULL;
#endif
}
