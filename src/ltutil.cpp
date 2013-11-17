/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltutil)

#ifdef LTMINGW
#include <windows.h>
#include <shlobj.h>
#endif

#ifdef LTANDROID
const char *lt_android_data_dir = NULL;
#endif

void ltAbortImpl(const char *file, int line) {
    ltLog("%s:%d: ABORTING", file, line);
    exit(1);
}

#define MAX_MSG_LEN 102400

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
#ifndef LTMINGW
    // Non-windows
    const char *homedir;
    homedir = getenv("HOME");
    /*
     * Commented out to avoid warnings when statically linking.
    if (homedir == NULL) {
        struct passwd *pw = getpwuid(getuid());
        if (pw != NULL) {
            homedir = pw->pw_dir;
        }
    }
    */
    if (homedir == NULL) {
        ltLog("Unable to find your home directory. Please set the HOME env var.");
        ltAbort();
    }
    if (!ltFileExists(homedir)) {
        ltLog("Home directory '%s' does not exist.", homedir);
        ltAbort();
    }
    return homedir;
#else
    // Windows
    static char homedir[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, homedir) != S_OK) {
        ltLog("Unable to get home directory.");
        ltAbort();
    }
    return homedir;
#endif
}

const char *ltAppDataDir() {
    const char *short_name = lt_app_short_name;
    if (short_name == NULL || strlen(short_name) == 0) {
#ifdef LTDEVMODE
        short_name = "ltclient";
#else
        ltLog("lt_app_short_name not set");
        ltAbort();
#endif
    }
#if defined(LTLINUX) || defined(LTOSX)
    static char appdata_dir[1024];
    static bool initialized = false;
    if (!initialized) {
        snprintf(appdata_dir, 1024, "%s/.%s", ltHomeDir(), short_name);
        ltMkDir(appdata_dir);
        initialized = true;
    }
    return appdata_dir;
#elif LTMINGW
    static char win_appdata_dir[MAX_PATH];
    static char appdata_dir[MAX_PATH];
    static bool initialized = false;
    if (!initialized) {
        if (SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, win_appdata_dir) != S_OK) {
            ltLog("Unable to get appdata directory");
            ltAbort();
        }
        snprintf(appdata_dir, MAX_PATH, "%s/%s", win_appdata_dir, short_name);
        ltMkDir(appdata_dir);
        initialized = true;
    }
    return appdata_dir;
#elif LTANDROID
    return lt_android_data_dir;
#elif LTTIZEN
    Tizen::App::App *app = Tizen::App::App::GetInstance();
    Tizen::Base::String path = app->GetAppDataPath();
    Tizen::Base::ByteBuffer *bb = Tizen::Base::Utility::StringUtil::StringToUtf8N(path);
    const char *ptr = (const char*)bb->GetPointer();
    if (ptr == NULL) {
        ltLog("Error: GetAppDataPath returned NULL");
        ltAbort();
        return NULL;
    } else {
        char *dir = new char[strlen(ptr) + 1];
        strcpy(dir, ptr);
        delete bb;
        return dir;
    }
#else
    ltLog("ltAppDataDir NYI");
    ltAbort();
    return NULL;
#endif
}

void ltMkDir(const char* dir) {
    if (!ltFileExists(dir)) {
#ifdef LTMINGW
        int r = mkdir(dir);
#else
        int r = mkdir(dir, S_IRWXU | S_IRWXG);
#endif
        if (r < 0) {
            ltLog("Error creating directory %s: %s.  Aborting.", dir, strerror(errno));
        }
    }
}

void ltSleep(int millisecs) {
    #ifdef LTMINGW
        Sleep(millisecs);
    #else
        usleep(millisecs * 1000);
    #endif
}
