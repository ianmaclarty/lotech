/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTLUA_H
#define LTLUA_H

#include "ltinput.h"

void ltLuaSetup(const char *file);
void ltLuaTeardown();

void ltLuaAdvance();
void ltLuaRender();

void ltLuaKeyDown(LTKey);
void ltLuaKeyUp(LTKey);
void ltLuaMouseDown(int button, LTfloat x, LTfloat y);
void ltLuaMouseUp(int button, LTfloat x, LTfloat y);
void ltLuaMouseMove(LTfloat x, LTfloat y);
void ltLuaResizeWindow(LTfloat w, LTfloat h);

#endif /* LTLUA_H */
