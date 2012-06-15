/* Copyright (C) 2010 Ian MacLarty */
LT_INIT_DECL(ltstate)

// Save the lt.state lua table to persistent storage.
void ltSaveState();

// Restore the lt.state lua table from persistent storage.
void ltRestoreState();
