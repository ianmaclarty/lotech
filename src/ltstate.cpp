#include "lt.h"

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
    #endif
}
