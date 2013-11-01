/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltresource)

#ifdef LTANDROID
struct LTResource {
    AAsset *asset;
    char *name;
};
void ltSetAssetManager(AAssetManager* mgr);
#else
struct LTResource {
    FILE *file;
    char *name;
};
#endif

// Returns NULL on failure.
LTResource *ltOpenResource(const char* filename);

bool ltResourceExists(const char* filename);

// Returns bytes read, < 0 on error.
int ltReadResource(LTResource *rsc, void* buf, int count);

void ltCloseResource(LTResource *rsc);

// Free with free()
char* ltReadTextResource(const char *filename, int *len);
void* ltReadResourceAll(LTResource *rsc, int *size);

const char *ltResourcePath(const char *resource, const char *suffix);

void ltSetResourcePrefix(const char *prefix);

