/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTSTATE_H
#define LTSTATE_H

// Save the lt.state lua table to persistent storage.
void ltSaveState();

// Restore the lt.state lua table from persistent storage.
void ltRestoreState();

#endif
