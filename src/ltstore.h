/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltstore)

void ltStorePickledData(const char *key, LTPickler *pickler);

// The caller must free the returned unpickler with delete.
LTUnpickler* ltRetrievePickledData(const char *key);

void ltUnstore(const char *key);

// Everything below this line is deprecated.

enum LTStoredValueType {
    LT_STORED_VALUE_TYPE_STRING,
    LT_STORED_VALUE_TYPE_DOUBLE,
    LT_STORED_VALUE_TYPE_FLOAT,
    LT_STORED_VALUE_TYPE_INT,
    LT_STORED_VALUE_TYPE_BOOL,
    LT_STORED_VALUE_TYPE_UNKNOWN,
};

void ltStoreString(const char *key, const char *value);
void ltStoreDouble(const char *key, LTdouble value);
void ltStoreFloat(const char *key, LTfloat value);
void ltStoreInt(const char *key, int value);
void ltStoreBool(const char *key, bool value);
LTStoredValueType ltGetStoredValueType(const char *key);

// The caller must free the returned value with delete[].
char* ltRetrieveString(const char *key);
LTdouble ltRetrieveDouble(const char *key);
LTfloat ltRetrieveFloat(const char *key);
int ltRetrieveInt(const char *key);
bool ltRetrieveBool(const char *key);
