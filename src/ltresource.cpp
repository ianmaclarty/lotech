/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltresource)

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

#endif

char* ltReadTextResource(const char *path) {
    LTResource *rsc = ltOpenResource(path);
    if (rsc == NULL) {
        return NULL;
    }
    int capacity = 1024;
    char *buf = (char*)malloc(capacity);
    char *ptr = buf;
    int total = 0;
    while (true) {
        int n = ltReadResource(rsc, ptr, capacity - total);
        total += n;
        if (n >= 0 && n < capacity - total) {
            buf[total] = '\0';
            ltCloseResource(rsc);
            return buf;
        } else if (n >= 0) {
            capacity *= 2;
            buf = (char*)realloc(buf, capacity);
            ptr = buf + total;
        } else {
            // error
            ltCloseResource(rsc);
            return NULL;
        }
    }
}
