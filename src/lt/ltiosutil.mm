#ifdef LTIOS
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <objc/runtime.h>

#include "ltiosutil.h"
#include "ltutil.h"

LTfloat ltIOSScaling() {
    float scale = 1.0f;
    if([[UIScreen mainScreen] respondsToSelector: NSSelectorFromString(@"scale")]) {
        scale = [[UIScreen mainScreen] scale];
    }
    return scale;
}

bool ltIsIPad() {
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
        return true;
    } else {
        return false;
    }
}

bool ltIsRetinaIPhone() {
    return !ltIsIPad() && ltIOSScaling() == 2.0f;
}

const char *ltIOSDocPath(const char *file, const char *suffix) {
    NSArray *dirs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    const char *dir = [[dirs objectAtIndex:0] UTF8String];
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
    return path;
}

const char *ltIOSBundlePath(const char *file, const char *suffix) {
    #ifdef LTDEVMODE
    const char *docpath = ltIOSDocPath(file, suffix);
    #endif
    const char *dir = [[[NSBundle mainBundle] bundlePath] UTF8String];
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

void ltIOSStorePickledData(const char *key, LTPickler *pickler) {
    ensure_prefs_initialized();
    NSData *nsdata = [NSData dataWithBytesNoCopy:pickler->data length:pickler->size freeWhenDone:NO];
    NSString *skey = [NSString stringWithUTF8String:key];
    [prefs setObject:nsdata forKey:skey];
}

LTUnpickler *ltIOSRetrievePickledData(const char *key) {
    ensure_prefs_initialized();
    NSData *nsdata = [prefs dataForKey:[NSString stringWithUTF8String:key]];
    if (nsdata != nil) {
        LTUnpickler *unpickler = new LTUnpickler(nsdata.bytes, nsdata.length);
        return unpickler;
    } else {
        return NULL;
    }
}

void ltIOSStoreString(const char *key, const char *value) {
    ensure_prefs_initialized();
    NSString *skey = [NSString stringWithUTF8String:key];
    NSString *svalue = [NSString stringWithUTF8String:value];
    [prefs setObject:svalue forKey:skey];
}

char *ltIOSRetrieveString(const char *key) {
    ensure_prefs_initialized();
    NSString *svalue = [prefs stringForKey:[NSString stringWithUTF8String:key]];
    const char *cstr;
    if (svalue != nil) {
        cstr = [svalue UTF8String];
    } else {
        cstr = "";
    }
    char *rv = new char[strlen(cstr) + 1];
    strcpy(rv, cstr);
    return rv;
}

void ltIOSStoreDouble(const char *key, LTdouble value) {
    ensure_prefs_initialized();
    NSString *skey = [NSString stringWithUTF8String:key];
    [prefs setDouble:value forKey:skey];
}

LTdouble ltIOSRetrieveDouble(const char *key) {
    ensure_prefs_initialized();
    return [prefs doubleForKey:[NSString stringWithUTF8String:key]];
}

void ltIOSStoreFloat(const char *key, LTfloat value) {
    ensure_prefs_initialized();
    NSString *skey = [NSString stringWithUTF8String:key];
    [prefs setFloat:value forKey:skey];
}

LTfloat ltIOSRetrieveFloat(const char *key) {
    ensure_prefs_initialized();
    return [prefs floatForKey:[NSString stringWithUTF8String:key]];
}

void ltIOSStoreInt(const char *key, int value) {
    ensure_prefs_initialized();
    NSString *skey = [NSString stringWithUTF8String:key];
    [prefs setInteger:value forKey:skey];
}

int ltIOSRetrieveInt(const char *key) {
    ensure_prefs_initialized();
    return [prefs integerForKey:[NSString stringWithUTF8String:key]];
}

void ltIOSStoreBool(const char *key, bool value) {
    ensure_prefs_initialized();
    NSString *skey = [NSString stringWithUTF8String:key];
    [prefs setBool:(value ? YES : NO) forKey:skey];
}

bool ltIOSRetrieveBool(const char *key) {
    ensure_prefs_initialized();
    BOOL b = [prefs integerForKey:[NSString stringWithUTF8String:key]];
    return (b == NO ? false : true);
}

LTStoredValueType ltIOSGetStoredValueType(const char *key) {
    ensure_prefs_initialized();
    id obj = [prefs objectForKey:[NSString stringWithUTF8String:key]];
    if (obj == nil) {
        return LT_STORED_VALUE_TYPE_UNKNOWN;
    }
    const char *cname = object_getClassName(obj);
    if (cname != NULL) {
        if (strcmp(cname, "NSCFNumber") == 0) {
            NSNumber *num = (NSNumber*)obj;
            const char *type = [num objCType];
            switch (*type) {
                case 'd': return LT_STORED_VALUE_TYPE_DOUBLE;
                case 'f': return LT_STORED_VALUE_TYPE_FLOAT;
                case 'i': return LT_STORED_VALUE_TYPE_INT;
                default: return LT_STORED_VALUE_TYPE_UNKNOWN;
            }
        } else if (strcmp(cname, "NSCFString") == 0) {
            return LT_STORED_VALUE_TYPE_STRING;
        } else if (strcmp(cname, "NSCFBoolean") == 0) {
            return LT_STORED_VALUE_TYPE_BOOL;
        }
    }
    return LT_STORED_VALUE_TYPE_UNKNOWN;
}

void ltIOSUnstore(const char *key) {
    ensure_prefs_initialized();
    [prefs removeObjectForKey:[NSString stringWithUTF8String:key]];
}

void ltIOSSyncStore() {
    if (prefs != nil) {
        [prefs synchronize];
    }
}

void ltIOSLaunchURL(const char *url) {
    NSString *ns_url = [NSString stringWithUTF8String:url];
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:ns_url]];
}

bool ltIOSSupportsES2() {
    EAGLContext *testContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    bool isSGX = testContext ? true : false;
    [testContext release];
    return isSGX;
}

#endif //LTIOS
