/* Copyright (C) 2011 Ian MacLarty */
#ifdef LTOSX
#ifndef LTOSX_H
#define LTOSX_H

#import <AppKit/AppKit.h>

void ltOSXInit();
void ltOSXTeardown();

void ltOSXResize(int width, int height);

void ltOSXAdvance(float secs);
void ltOSXRender();

void ltOSXMouseDown(NSEvent *event, NSView *view);
void ltOSXMouseUp(NSEvent *event, NSView *view);
void ltOSXMouseMoved(NSEvent *event, NSView *view);
void ltOSXKeyUp(NSEvent *event);
void ltOSXKeyDown(NSEvent *event);

void ltOSXSaveState();

//void ltOSXKeyDown(NSEvent *event);
//void ltOSXKeyUp(NSEvent *event);

#endif
#endif
