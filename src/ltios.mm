/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#ifdef LTIOS
#include "lt.h"

static UIViewController *view_controller = nil;

// The following is required for converting UITouch objects to input_ids.
ct_assert(sizeof(UITouch*) == sizeof(int));

void ltIOSInit() {
    #ifdef LTDEVMODE
    ltClientInit();
    #endif

    ltLuaSetup();
}

void ltIOSSetViewController(UIViewController *view_c) {
    view_controller = view_c;
    [view_controller retain];

    #ifdef LTGAMECENTER
    ltIOSInitGameCenter();
    #endif

    #ifdef LTADS 
    ltShowAds(LTADS);
    #endif
}

void ltIOSTeardown() {
    #ifdef LTGAMECENTER
    ltIOSTeardownGameCenter();
    #endif
    ltLuaTeardown();
    if (view_controller != nil) {
        [view_controller release];
    }
}

void ltIOSResize(int width, int height) {
    ltResizeScreen(width, height);
}

void ltIOSRender() {
    ltLuaAdvance(1.0f/60.0f);
    ltLuaRender();

    #ifdef LTDEVMODE
    ltClientStep();
    #endif
}

void ltIOSGarbageCollect() {
    ltLuaGarbageCollect();
}

void ltIOSTouchesBegan(NSSet *touches) {
    NSEnumerator *e = [touches objectEnumerator];
    UITouch *touch;
    while ((touch = [e nextObject])) {
        CGPoint pos = [touch locationInView:touch.view];
        ltLuaTouchDown((int)touch, pos.x, pos.y);
    }
}

void ltIOSTouchesMoved(NSSet *touches) {
    NSEnumerator *e = [touches objectEnumerator];
    UITouch *touch;
    while ((touch = [e nextObject])) {
        CGPoint pos = [touch locationInView:touch.view];
        ltLuaTouchMove((int)touch, pos.x, pos.y);
    }
}

void ltIOSTouchesEnded(NSSet *touches) {
    NSEnumerator *e = [touches objectEnumerator];
    UITouch *touch;
    while ((touch = [e nextObject])) {
        CGPoint pos = [touch locationInView:touch.view];
        ltLuaTouchUp((int)touch, pos.x, pos.y);
    }
}

void ltIOSTouchesCancelled(NSSet *touches) {
    ltIOSTouchesEnded(touches);
}

UIViewController *ltIOSGetViewController() {
    return view_controller;
}

void ltIOSSaveState() {
    ltSaveState();
    ltIOSSyncStore();
}

void ltIOSResignActive() {
#ifdef LTDEVMODE
    ltClientShutdown();
#endif
}

void ltIOSBecomeActive() {
#ifdef LTDEVMODE
    ltClientInit();
#endif
}

#endif // LTIOS
