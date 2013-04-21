/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#ifdef LTOSX

const char *ltOSXBundlePath(const char *file, const char *suffix);

void ltOSXStorePickledData(const char *key, LTPickler *pickler);
LTUnpickler *ltOSXRetrievePickledData(const char *key);

void ltOSXSyncStore();

#endif
