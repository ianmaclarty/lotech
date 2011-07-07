#ifdef LTIOS
#include "ltads.h"
#include "ltios.h"
#include "ltcommon.h"
#include "ltgraphics.h"
#include "ltgamecenter.h"
#include "ltiosutil.h"
#include "ltlua.h"
#include "ltprotocol.h"

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
    ltLuaAdvance();
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
    while (touch = [e nextObject]) {
        CGPoint pos = [touch locationInView:touch.view];
        ltLuaPointerDown((int)touch, pos.x, pos.y);
    }
}

void ltIOSTouchesMoved(NSSet *touches) {
    NSEnumerator *e = [touches objectEnumerator];
    UITouch *touch;
    while (touch = [e nextObject]) {
        CGPoint pos = [touch locationInView:touch.view];
        ltLuaPointerMove((int)touch, pos.x, pos.y);
    }
}

void ltIOSTouchesEnded(NSSet *touches) {
    NSEnumerator *e = [touches objectEnumerator];
    UITouch *touch;
    while (touch = [e nextObject]) {
        CGPoint pos = [touch locationInView:touch.view];
        ltLuaPointerUp((int)touch, pos.x, pos.y);
    }
}

void ltIOSTouchesCancelled(NSSet *touches) {
    ltIOSTouchesEnded(touches);
}

UIViewController *ltIOSGetViewController() {
    return view_controller;
}

#endif // LTIOS
