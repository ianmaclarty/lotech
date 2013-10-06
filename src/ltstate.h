/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltstate)

// Save the lt.state lua table to persistent storage.
void ltSaveState();

// Restore the lt.state lua table from persistent storage.
void ltRestoreState();

void ltSetAndroidUnpickler(LTUnpickler *unpickler);
