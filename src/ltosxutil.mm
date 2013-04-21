/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#ifdef LTOSX
#include "lt.h"

static const char *ltOSXDocPath(const char *file, const char *suffix) {
    NSArray *dirs = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *nsdir = [[dirs objectAtIndex:0] stringByAppendingString:@"/Lotech"];
    const char *cdir = [nsdir UTF8String];

    NSFileManager *fileManager = [NSFileManager defaultManager]; 
    [fileManager createDirectoryAtPath:nsdir withIntermediateDirectories:YES attributes:nil error:NULL];

    int len = strlen(cdir) + strlen(file) + 2;
    if (suffix != NULL) {
        len += strlen(suffix);
    }
    char *path = new char[len];
    if (suffix == NULL) {
        snprintf(path, len, "%s/%s", cdir, file);
    } else {
        snprintf(path, len, "%s/%s%s", cdir, file, suffix);
    }
    return path;
}

const char *ltOSXBundlePath(const char *file, const char *suffix) {
    #ifdef LTDEVMODE
    const char *docpath = ltOSXDocPath(file, suffix);
    #endif
    const char *dir = [[[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/Contents/Resources"] UTF8String];
    int len = strlen(dir) + strlen(file) + 2;
    if (suffix != NULL) {
        len += strlen(suffix);
    }
    char *path = new char[len];
    if (suffix == NULL) {
        snprintf(path, len, "%s/%s", dir, file);
    } else {
        snprintf(path, len, "%s/%s%s", dir, file, suffix);
    }
    #ifdef LTDEVMODE
    if (ltFileExists(docpath) || !ltFileExists(path)) {
        delete[] path;
        path = (char*)docpath;
    } else {
        delete[] docpath;
    }
    #endif
    return path;
}

//------------- prefs store ----------------

static NSUserDefaults *prefs = nil;

static void ensure_prefs_initialized() {
    if (prefs == nil) {
        prefs = [NSUserDefaults standardUserDefaults];
    }
}

void ltOSXStorePickledData(const char *key, LTPickler *pickler) {
    ensure_prefs_initialized();
    NSData *nsdata = [NSData dataWithBytesNoCopy:pickler->data length:pickler->size freeWhenDone:NO];
    NSString *skey = [NSString stringWithUTF8String:key];
    [prefs setObject:nsdata forKey:skey];
}

LTUnpickler *ltOSXRetrievePickledData(const char *key) {
    ensure_prefs_initialized();
    NSData *nsdata = [prefs dataForKey:[NSString stringWithUTF8String:key]];
    if (nsdata != nil) {
        LTUnpickler *unpickler = new LTUnpickler(nsdata.bytes, nsdata.length);
        return unpickler;
    } else {
        return NULL;
    }
}

void ltOSXSyncStore() {
    if (prefs != nil) {
        [prefs synchronize];
    }
}
#endif
