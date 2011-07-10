#include "ltlua.h"
#include "ltstore.h"

void ltSaveState() {
    LTPickler *pickler = ltLuaPickleState();
    if (pickler != NULL) {
        ltStorePickledData("ltstate", pickler);
        delete pickler;
    }
}

void ltRestoreState() {
    LTUnpickler *unpickler = ltRetrievePickledData("ltstate");
    if (unpickler != NULL) {
        ltLuaUnpickleState(unpickler);
        delete unpickler;
    }
}
