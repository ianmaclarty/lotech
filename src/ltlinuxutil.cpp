#include "lt.h"

static const char* pickle_file_name(const char *key) {
    const char *homedir = ltHomeDir();
    if (lt_app_short_name == NULL) {
        ltLog("ERROR: lt_app_short_name not set.");
        ltAbort();
    }
    if (key == NULL || strlen(key) == 0) {
        ltLog("pickle_file_name: key null or empty");
        ltAbort();
    }
    char path[1024];
    snprintf(path, 1024, "%s/.%s", homedir, lt_app_short_name);
    ltMkDir(path);
    snprintf(path, 1024, "%s/.%s/%s", homedir, lt_app_short_name, key);
    char *filename = new char[strlen(path)];
    strcpy(filename, path);
    return filename;
}

void ltLinuxStorePickledData(const char *key, LTPickler *pickler) {
    const char *filename = pickle_file_name(key);
    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        ltLog("Error opening %s for writing: %s.", filename, strerror(errno));
        ltAbort();
    }
    delete[] filename;
    LTint32 size = (LTint32)pickler->size;
    fwrite(&size, 4, 1, f);
    fwrite(pickler->data, size, 1, f);
    fclose(f);
}

LTUnpickler *ltLinuxRetrievePickledData(const char *key) {
    const char *filename = pickle_file_name(key);
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        delete[] filename;
        return NULL;
    }
    LTint32 size;
    int n = fread(&size, 1, 4, f);
    if (n < 4) {
        ltLog("Error reading header from %s (only got %d bytes, expecting 4).", filename, n);
        delete[] filename;
        ltAbort();
    }
    void *data = malloc(size);
    n = fread(data, 1, size, f);
    if (n < size) {
        ltLog("Error reading data from %s (only got %d bytes, expecting %d).", filename, n, size);
        delete[] filename;
        ltAbort();
    }
    LTUnpickler *unpickler = new LTUnpickler(data, size);
    free(data);
    delete[] filename;
    return unpickler;
}
