#include "lt.h"

#ifndef LTJS

#define CACHET std::map<char *, char *, CstrCmp>

struct CstrCmp {
    bool operator()(char* a, char* b) {
            return strcmp(a, b) < 0;
    }
};

static CACHET cache;

void ltLuaCacheAdd(const char *path, const char *data) {
    char *path_copy = new char[strlen(path) + 1];
    char *data_copy = new char[strlen(data) + 1];
    strcpy(path_copy, path);
    strcpy(data_copy, data);
    cache[path_copy] = data_copy;
}

const char *ltLuaReadCache(const char *path) {
    CACHET::iterator it = cache.find((char*)path);
    if (it == cache.end()) {
        return NULL;
    } else {
        return it->second;
    }
}

#else

void ltLuaCacheAdd(const char *path, const char *data) {
}

const char *ltLuaReadCache(const char *path) {
    return NULL;
}

#endif
