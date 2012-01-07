/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTLUA_H
#define LTLUA_H

#include "ltcommon.h"
#include "ltinput.h"
#include "ltpickle.h"

void ltLuaSetup();
void ltLuaTeardown();
void ltLuaReset();
void ltLuaSuspend();
void ltLuaResume();

void ltLuaAdvance(LTfloat secs);
void ltLuaRender();

void ltLuaKeyDown(LTKey);
void ltLuaKeyUp(LTKey);

void ltLuaPointerDown(int input_id, LTfloat x, LTfloat y);
void ltLuaPointerUp(int input_id, LTfloat x, LTfloat y);
void ltLuaPointerMove(int input_id, LTfloat x, LTfloat y);

void ltLuaResizeWindow(LTfloat w, LTfloat h);

void ltLuaGameCenterBecameAvailable();

void ltLuaGarbageCollect();

int ltLuaInitRef();

// The caller should free the pickler with delete.
LTPickler *ltLuaPickleState();
void ltLuaUnpickleState(LTUnpickler *unpickler);
#endif /* LTLUA_H */
