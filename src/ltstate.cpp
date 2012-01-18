#include "ltlua.h"
#include "ltstore.h"

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
    if (unpickler != NULL) {
        ltLuaUnpickleState(unpickler);
        delete unpickler;
    }
    #endif
}
