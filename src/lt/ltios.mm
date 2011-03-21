#include "ltios.h"
#include "ltcommon.h"
#include "ltgraphics.h"
#include "ltiosutil.h"
#include "ltlua.h"
#include "ltprotocol.h"

static UIViewController *view_controller = nil;

// The following is required for converting UITouch objects to input_ids.
ct_assert(sizeof(UITouch*) == sizeof(int));

void ltIOSInit(UIViewController *vc) {
    view_controller = vc;

    #ifdef LTDEVMODE
    ltClientInit();
    #endif

    ltSetScreenSize(ltIOSScreenWidth(), ltIOSScreenHeight());

    const char *path = ltIOSBundlePath("main", ".lua");
    ltLuaSetup(path);
    delete[] path;
}

void ltIOSTeardown() {
    ltLuaTeardown();
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
    [touches enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
        UITouch *touch = (UITouch*)obj;
        CGPoint pos = [touch locationInView:touch.view];
        ltLuaPointerDown((int)touch, pos.x, pos.y);
    }];
}

void ltIOSTouchesMoved(NSSet *touches) {
    [touches enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
        UITouch *touch = (UITouch*)obj;
        CGPoint pos = [touch locationInView:touch.view];
        ltLuaPointerMove((int)touch, pos.x, pos.y);
    }];
}

void ltIOSTouchesEnded(NSSet *touches) {
    [touches enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
        UITouch *touch = (UITouch*)obj;
        CGPoint pos = [touch locationInView:touch.view];
        ltLuaPointerUp((int)touch, pos.x, pos.y);
    }];
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

int ltIOSScreenWidth() {
    float scale = scaling();
    int w = view_controller.view.bounds.size.width * scale;
    return w;
}

int ltIOSScreenHeight() {
    float scale = scaling();
    return view_controller.view.bounds.size.height * scale;
}

