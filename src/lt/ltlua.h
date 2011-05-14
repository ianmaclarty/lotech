/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTLUA_H
#define LTLUA_H

#include "ltinput.h"

void ltLuaSetup();
void ltLuaTeardown();
void ltLuaReset();

void ltLuaAdvance();
void ltLuaRender();

void ltLuaKeyDown(LTKey);
void ltLuaKeyUp(LTKey);
void ltLuaPointerDown(int input_id, LTfloat x, LTfloat y);
void ltLuaPointerUp(int input_id, LTfloat x, LTfloat y);
void ltLuaPointerMove(int input_id, LTfloat x, LTfloat y);
void ltLuaResizeWindow(LTfloat w, LTfloat h);

void ltLuaGarbageCollect();

int ltLuaInitRef();

#endif /* LTLUA_H */
