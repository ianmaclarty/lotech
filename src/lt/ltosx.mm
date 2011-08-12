#ifdef LTOSX
#include "ltosx.h"
#include "ltosxutil.h"
#include "ltcommon.h"
#include "ltgraphics.h"
#include "ltlua.h"
#include "ltprotocol.h"
#include "ltstate.h"

static NSLock *mutex = [[NSLock alloc] init];

static void lock() {
    [mutex lock];
}

static void unlock() {
    [mutex unlock];
}

static int w, h, but;

void ltOSXInit() {
    lock();
    #ifdef LTDEVMODE
    ltClientInit();
    #endif

    ltLuaSetup();
    unlock();
}

void ltOSXTeardown() {
    lock();
    ltLuaTeardown();
    unlock();
}

void ltOSXResize(int width, int height) {
    lock();
    ltLuaResizeWindow(width, height);
    w = width;
    h = height;
    unlock();
}

void ltOSXAdvance(LTfloat secs) {
    lock();
    ltLuaAdvance(secs);
    #ifdef LTDEVMODE
    ltClientStep();
    #endif
    unlock();
}

void ltOSXRender() {
    lock();
    ltLuaRender();
    unlock();
}

static NSPoint get_pos(NSEvent *event, NSView *view) {
    if (view == nil) {
        return [event locationInWindow];
    } else {
        return [view convertPoint:[event locationInWindow] fromView:nil];
    }
}

void ltOSXMouseDown(NSEvent *event, NSView *view) {
    lock();
    NSPoint pos = get_pos(event, view);
    but = 1;
    ltLuaPointerDown(but, pos.x, h - pos.y);
    unlock();
}

void ltOSXMouseUp(NSEvent *event, NSView *view) {
    lock();
    NSPoint pos = get_pos(event, view);
    ltLuaPointerUp(but, pos.x, h - pos.y);
    but = 0;
    unlock();
}

void ltOSXMouseMoved(NSEvent *event, NSView *view) {
    lock();
    NSPoint pos = get_pos(event, view);
    ltLuaPointerMove(but, pos.x, h - pos.y);
    unlock();
}

void ltOSXSaveState() {
    lock();
    ltSaveState();
    ltOSXSyncStore();
    unlock();
}

#endif // LTOSX
