#ifdef LTIOS
#include "ltios.h"
#include "ltcommon.h"
#include "ltgraphics.h"
#include "ltiosutil.h"
#include "ltlua.h"
#include "ltprotocol.h"

// The following is required for converting UITouch objects to input_ids.
ct_assert(sizeof(UITouch*) == sizeof(int));

void ltIOSInit() {
    #ifdef LTDEVMODE
    ltClientInit();
    #endif

    ltLuaSetup();
}

void ltIOSTeardown() {
    ltLuaTeardown();
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
#endif // LTIOS
