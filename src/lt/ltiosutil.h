#ifdef LTIOS
#ifndef LTIOSUTIL_H
#define LTIOSUTIL_H

#include "ltcommon.h"
#include "ltstore.h"

bool ltIsIPad();
bool ltIsRetinaIPhone();

// The caller should free the returned string with delete[].
// If suffix is not null, it is appended to the end of the returned
// string.
const char *ltIOSBundlePath(const char *file, const char *suffix);

LTfloat ltIOSScaling();

void ltIOSStoreString(const char *key, const char *value);
// The caller is responsible for freeing the returned value with delete[].
char *ltIOSRetrieveString(const char *key);

void ltIOSStoreDouble(const char *key, LTdouble value);
LTdouble ltIOSRetrieveDouble(const char *key);

void ltIOSStoreFloat(const char *key, LTfloat value);
LTfloat ltIOSRetrieveFloat(const char *key);

void ltIOSStoreInt(const char *key, int value);
int ltIOSRetrieveInt(const char *key);

void ltIOSStoreBool(const char *key, bool value);
bool ltIOSRetrieveBool(const char *key);

LTStoredValueType ltIOSGetStoredValueType(const char *key);

void ltIOSUnstore(const char *key);

void ltIOSSyncStore();

#endif
#endif
