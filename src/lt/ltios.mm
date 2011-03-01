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
    ltSetScreenSize(ltIOSPortaitPixelWidth(), ltIOSPortaitPixelHeight());
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
    ltLuaPointerDown(0, 0.0f, 0.0f);
}
void ltIOSTouchesMoved(NSSet *touches) {
}
void ltIOSTouchesEnded(NSSet *touches) {
}
void ltIOSTouchesCancelled(NSSet *touches) {
    ltIOSTouchesEnded(touches);
}
