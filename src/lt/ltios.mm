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

    const char *path = ltIOSBundlePath("main", ".lua");
    if (ltFileExists(path)) {
        ltLuaSetup(path);
        delete[] path;
    } else {
        delete[] path;
        path = ltIOSBundlePath("lotech_default", ".lua");
        ltLuaSetup(path);
        delete[] path;
    }
}

void ltIOSTeardown() {
    ltLuaTeardown();
}

void ltIOSResize(int width, int height) {
    ltResizeScreen(width, height);
}

void ltIOSRender() {
    ltLuaRender();
    ltLuaAdvance();

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

static float scaling() {
    float scale = 1.0f;
    if([[UIScreen mainScreen] respondsToSelector: NSSelectorFromString(@"scale")]) {
        scale = [[UIScreen mainScreen] scale];
    }
    return scale;
}
