/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltstate)

#ifdef LTANDROID
static LTUnpickler *android_unpickler = NULL;
void ltSetAndroidUnpickler(LTUnpickler *unpickler) {
    if (android_unpickler != NULL) delete android_unpickler;
    android_unpickler = unpickler;
}
#endif

void ltSaveState() {
    #ifndef LTANDROID
    LTPickler *pickler = ltLuaPickleState();
    if (pickler != NULL) {
        ltStorePickledData("ltstate", pickler);
        delete pickler;
    }
    #endif
}

void ltRestoreState() {
    #ifndef LTANDROID
    LTUnpickler *unpickler = ltRetrievePickledData("ltstate");
    ltLuaUnpickleState(unpickler);
    if (unpickler != NULL) {
        delete unpickler;
    }
    #else
    ltLuaUnpickleState(android_unpickler);
    #endif
}
