/* Copyright (C) 2010 Ian MacLarty */
LT_INIT_DECL(ltlua)

void ltLuaSetup();
void ltLuaTeardown();
void ltLuaReset();
void ltLuaSuspend();
void ltLuaResume();

void ltLuaAdvance(LTdouble secs);
void ltLuaRender();

void ltLuaKeyDown(LTKey);
void ltLuaKeyUp(LTKey);

void ltLuaPointerDown(int input_id, LTfloat x, LTfloat y);
void ltLuaPointerUp(int input_id, LTfloat x, LTfloat y);
void ltLuaPointerMove(int input_id, LTfloat x, LTfloat y);

void ltLuaResizeWindow(LTfloat w, LTfloat h);

void ltLuaGameCenterBecameAvailable();

void ltLuaGarbageCollect();

void ltLuaSetResourcePrefix(const char *prefix);

// The caller should free the pickler with delete.
LTPickler *ltLuaPickleState();
void ltLuaUnpickleState(LTUnpickler *unpickler);

void ltLuaPreContextChange();
void ltLuaPostContextChange();
