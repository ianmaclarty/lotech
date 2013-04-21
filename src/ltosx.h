/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#ifdef LTOSX

void ltOSXInit();
void ltOSXTeardown();

void ltOSXResize(int width, int height);

void ltOSXRender();

void ltOSXMouseDown(NSEvent *event, NSView *view);
void ltOSXMouseUp(NSEvent *event, NSView *view);
void ltOSXMouseMoved(NSEvent *event, NSView *view);
void ltOSXKeyUp(NSEvent *event);
void ltOSXKeyDown(NSEvent *event);

void ltOSXSaveState();

void ltOSXPreContextChange();
void ltOSXPostContextChange();

#endif
