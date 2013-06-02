/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#ifdef LTIOS
bool ltIsIPad();
bool ltIsRetinaIPhone();

// The caller should free the returned string with delete[].
// If suffix is not null, it is appended to the end of the returned
// string.
const char *ltIOSBundlePath(const char *file, const char *suffix);

LTfloat ltIOSScaling();

void ltIOSStorePickledData(const char *key, LTPickler *pickler);
LTUnpickler *ltIOSRetrievePickledData(const char *key);

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

void ltIOSLaunchURL(const char *url);

bool ltIOSSupportsES2();

double ltIOSGetTime();
#endif
