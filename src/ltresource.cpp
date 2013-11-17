/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltresource)

static const char* resource_prefix = "";

#ifdef LTANDROID

static AAssetManager *asset_mgr = NULL;

void ltSetAssetManager(AAssetManager* mgr) {
    asset_mgr = mgr;
}

LTResource *ltOpenResource(const char* filename) {
    AAsset* asset = AAssetManager_open(asset_mgr, filename, AASSET_MODE_STREAMING);
    if (asset == NULL) {
        return NULL;
    }
    LTResource *rsc = new LTResource();
    rsc->asset = asset;
    rsc->name = new char[strlen(filename) + 1];
    strcpy(rsc->name, filename);
    return rsc;
}

int ltReadResource(LTResource *rsc, void* buf, int count) {
    int n = AAsset_read(rsc->asset, buf, count);
    return n;
}

void ltCloseResource(LTResource *rsc) {
    AAsset_close(rsc->asset);
    delete[] rsc->name;
    delete rsc;
}

bool ltResourceExists(const char* filename) {
    // XXX What's the overhead of this?
    AAsset* asset = AAssetManager_open(asset_mgr, filename, AASSET_MODE_STREAMING);
    if (asset != NULL) {
        AAsset_close(asset);
        return true;
    } else {
        return false;
    }
}

#else

LTResource *ltOpenResource(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (f == NULL) {
        return NULL;
    }
    LTResource *rsc = new LTResource();
    rsc->file = f;
    rsc->name = new char[strlen(filename) + 1];
    strcpy(rsc->name, filename);
    return rsc;
}

int ltReadResource(LTResource *rsc, void* buf, int count) {
    int n = fread(buf, 1, count, rsc->file);
    if (n < count && ferror(rsc->file)) {
        clearerr(rsc->file);
        return -1;
    } else {
        return n;
    }
}

void ltCloseResource(LTResource *rsc) {
    fclose(rsc->file);
    delete[] rsc->name;
    delete rsc;
}

bool ltResourceExists(const char* filename) {
    return ltFileExists(filename);
}

#endif

char* ltReadTextResource(const char *path, int *len) {
    LTResource *rsc = ltOpenResource(path);
    if (rsc == NULL) {
        return NULL;
    }
    int capacity = 1024;
    char *buf = (char*)malloc(capacity);
    char *ptr = buf;
    *len = 0;
    while (true) {
        int n = ltReadResource(rsc, ptr, capacity - *len);
        *len += n;
        if (n >= 0 && n < capacity - *len) {
            buf[*len] = '\0';
            ltCloseResource(rsc);
            return buf;
        } else if (n >= 0) {
            capacity *= 2;
            buf = (char*)realloc(buf, capacity);
            ptr = buf + *len;
        } else {
            // error
            ltCloseResource(rsc);
            return NULL;
        }
    }
}

void* ltReadResourceAll(LTResource *rsc, int *size) {
    int capacity = 1024;
    char *buf = (char*)malloc(capacity);
    char *ptr = buf;
    int total = 0;
    while (true) {
        int n = ltReadResource(rsc, ptr, capacity - total);
        total += n;
        if (n >= 0 && n < capacity - total) {
            *size = total;
            return buf;
        } else if (n >= 0) {
            capacity *= 2;
            buf = (char*)realloc(buf, capacity);
            ptr = buf + total;
        } else {
            // error
            return NULL;
        }
    }
}

void ltSetResourcePrefix(const char *prefix) {
    resource_prefix = prefix;
}

const char *ltResourcePath(const char *resource, const char *suffix) {
    const char *path;
    #ifdef LTIOS
        path = ltIOSBundlePath(resource, suffix);
    #elif LTOSX
        if (strlen(resource_prefix) == 0) {
            path = ltOSXBundlePath(resource, suffix);
        } else {
            int len = strlen(resource_prefix) + strlen(resource) + strlen(suffix) + 3;
            const char *slash = "";
            if (resource_prefix[strlen(resource_prefix) - 1] != '/') {
                len += 1;
                slash = "/";
            }
            path = new char[len];
            snprintf((char*)path, len, "%s%s%s%s", resource_prefix, slash, resource, suffix);
        }
    #elif LTANDROID
        int len = strlen(resource) + strlen(suffix) + 1;
        path = new char[len];
        snprintf((char*)path, len, "%s%s", resource, suffix);
    #elif LTTIZEN
        Tizen::App::App *app = Tizen::App::App::GetInstance();
        Tizen::Base::String str = app->GetAppResourcePath();
        Tizen::Base::ByteBuffer *bb = Tizen::Base::Utility::StringUtil::StringToUtf8N(str);
        const char *ptr = (const char*)bb->GetPointer();
        if (ptr == NULL) {
            ltLog("Error: GetAppResourcePath returned NULL");
            ltAbort();
        }
        char *dir = new char[strlen(ptr) + 1];
        strcpy(dir, ptr);
        delete bb;
        int len = strlen(dir) + 1 + strlen(resource) + strlen(suffix) + 1;
        path = new char[len];
        snprintf((char*)path, len, "%s/%s%s", dir, resource, suffix);
        delete[] dir;
    #else
        int len = strlen(resource_prefix) + strlen(resource) + strlen(suffix) + 3;
        const char *slash = "";
        if (resource_prefix[strlen(resource_prefix) - 1] != '/') {
            len += 1;
            slash = "/";
        }
        path = new char[len];
        snprintf((char*)path, len, "%s%s%s%s", resource_prefix, slash, resource, suffix);
    #endif
    return path;
}
