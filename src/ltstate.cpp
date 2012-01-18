#include "ltlua.h"
#include "ltstore.h"

void ltSaveState() {
    #ifndef LTANDROID
    LTPickler *pickler = ltLuaPickleState();
    if (pickler != NULL) {
        ltStorePickledData(pickler);
        delete pickler;
    }
    #endif
}

void ltRestoreState() {
    #ifndef LTANDROID
    LTUnpickler *unpickler = ltRetrievePickledData();
    if (unpickler != NULL) {
        ltLuaUnpickleState(unpickler);
        delete unpickler;
    }
    #endif
}
