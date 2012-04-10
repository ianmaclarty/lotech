/* Copyright (C) 2011 Ian MacLarty */
#ifdef LTOSX

const char *ltOSXBundlePath(const char *file, const char *suffix);

void ltOSXStorePickledData(const char *key, LTPickler *pickler);
LTUnpickler *ltOSXRetrievePickledData(const char *key);

void ltOSXSyncStore();

#endif
