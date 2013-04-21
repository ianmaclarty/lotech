/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltstate)

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
