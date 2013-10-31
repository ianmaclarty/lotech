/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltstate)

void ltSaveState() {
    LTPickler *pickler = ltLuaPickleState();
    if (pickler != NULL) {
        ltStorePickledData("ltstate", pickler);
        delete pickler;
    }
}

void ltRestoreState() {
    LTUnpickler *unpickler = ltRetrievePickledData("ltstate");
    ltLuaUnpickleState(unpickler);
    if (unpickler != NULL) {
        delete unpickler;
    }
}
