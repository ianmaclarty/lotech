/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltlua)

void ltLuaSetup();
void ltLuaTeardown();
void ltLuaReset();
void ltLuaSuspend();
void ltLuaResume();

void ltLuaAdvance(LTdouble secs);
void ltLuaRender();

void ltLuaKeyDown(LTKey key);
void ltLuaKeyUp(LTKey key);
void ltLuaTouchDown(int touch_id, LTfloat x, LTfloat y);
void ltLuaTouchUp(int touch_id, LTfloat x, LTfloat y);
void ltLuaTouchMove(int touch_id, LTfloat x, LTfloat y);
void ltLuaMouseDown(int button, LTfloat x, LTfloat y);
void ltLuaMouseUp(int button, LTfloat x, LTfloat y);
void ltLuaMouseMove(LTfloat x, LTfloat y);

void ltLuaResizeWindow(LTfloat w, LTfloat h);

void ltLuaGameCenterBecameAvailable();

void ltLuaGarbageCollect();

// The caller should free the pickler with delete.
LTPickler *ltLuaPickleState();
void ltLuaUnpickleState(LTUnpickler *unpickler);

void ltLuaPreContextChange();
void ltLuaPostContextChange();
