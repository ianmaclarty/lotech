LT_INIT_DECL(ltfilestore)

void ltStorePickledDataFile(const char *key, LTPickler *pickler);
LTUnpickler *ltRetrievePickledDataFile(const char *key);
