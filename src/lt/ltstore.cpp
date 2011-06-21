#include "ltstore.h"

#ifdef LTIOS
#include "ltiosutil.h"
#endif

void ltStoreString(const char *key, const char *value) {
    #ifdef LTIOS
        ltIOSStoreString(key, value);
    #endif
}

void ltStoreDouble(const char *key, LTdouble value) {
    #ifdef LTIOS
        ltIOSStoreDouble(key, value);
    #endif
}

void ltStoreFloat(const char *key, LTfloat value) {
    #ifdef LTIOS
        ltIOSStoreFloat(key, value);
    #endif
}

void ltStoreInt(const char *key, int value) {
    #ifdef LTIOS
        ltIOSStoreInt(key, value);
    #endif
}

void ltStoreBool(const char *key, bool value) {
    #ifdef LTIOS
        ltIOSStoreBool(key, value);
    #endif
}

LTStoredValueType ltGetStoredValueType(const char *key) {
    #ifdef LTIOS
        return ltIOSGetStoredValueType(key);
    #else
        return LT_STORED_VALUE_TYPE_UNKNOWN;
    #endif
}

char* ltRetrieveString(const char *key) {
    #ifdef LTIOS
        return ltIOSRetrieveString(key);
    #else
        char *rv = new char[1];
        *rv = '\0';
        return rv;
    #endif
}

LTdouble ltRetrieveDouble(const char *key) {
    #ifdef LTIOS
        return ltIOSRetrieveDouble(key);
    #else
        return 0.0;
    #endif
}

LTfloat ltRetrieveFloat(const char *key) {
    #ifdef LTIOS
        return ltIOSRetrieveFloat(key);
    #else
        return 0.0f;
    #endif
}

int ltRetrieveInt(const char *key) {
    #ifdef LTIOS
        return ltIOSRetrieveInt(key);
    #else
        return 0;
    #endif
}

bool ltRetrieveBool(const char *key) {
    #ifdef LTIOS
        return ltIOSRetrieveBool(key);
    #else
        return false;
    #endif
}

void ltUnstore(const char *key) {
    #ifdef LTIOS
        return ltIOSUnstore(key);
    #endif
}
