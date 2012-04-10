#include "lt.h"

void ltAbort() {
    ltLog("ABORTING.");
    exit(1);
}

#define MAX_MSG_LEN 2048

extern void ltLog(const char *fmt, ...) {
    char msg[MAX_MSG_LEN];
    va_list argp;
    va_start(argp, fmt);
    vsnprintf(msg, MAX_MSG_LEN, fmt, argp);
    va_end(argp);
#ifdef LTANDROID
    __android_log_print(ANDROID_LOG_INFO, "Lotech", "%s", msg);
#else
    fprintf(stderr, "%s\n", msg);
    fflush(stderr);
#endif
#ifdef LTDEVMODE
    if (ltAmClient()) {
        ltClientLog(msg);
    }    
#if defined LTLINUX || defined LTOSX
    /*
    FILE* log_file;
    log_file = fopen("/Users/ian/lt.log", "a");
    fprintf(log_file, "%s\n", msg);
    fflush(log_file);
    fcntl(fileno(log_file), F_FULLFSYNC);
    fclose(log_file);
    */
#endif
#endif
}

bool ltFileExists(const char *file) {
    struct stat info;
    return stat(file, &info) == 0;
}

char* ltGlob(const char **patterns) {
#if !defined LTANDROID && !defined LTMINGW
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
    ltLog("glob not supported on mingw or android");
    return NULL;
#endif
}

const char *ltHomeDir() {
    const char *homedir;
    homedir = getenv("HOME");
    if (homedir == NULL) {
        struct passwd *pw = getpwuid(getuid());
        if (pw != NULL) {
            homedir = pw->pw_dir;
        }
    }
    if (homedir == NULL) {
        ltLog("Unable to work out home directory.  Aborting.");
        ltAbort();
    }
    if (!ltFileExists(homedir)) {
        ltLog("Home directory '%s' does not exist.  Aborting.", homedir);
        ltAbort();
    }
    return homedir;
}

void ltMkDir(const char* dir) {
    if (!ltFileExists(dir)) {
        int r = mkdir(dir, S_IRWXU | S_IRWXG);
        if (r < 0) {
            ltLog("Error creating directory %s: %s.  Aborting.", dir, strerror(errno));
        }
    }
}
