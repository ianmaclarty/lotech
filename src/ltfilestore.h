/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltfilestore)

void ltStorePickledDataFile(const char *key, LTPickler *pickler);
LTUnpickler *ltRetrievePickledDataFile(const char *key);
