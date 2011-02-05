#include "ltcommon.h"
#include "ltgraphics.h"
#include "ltiosutil.h"
#include "ltlua.h"

// The following is required for converting UITouch objects to input_ids.
ct_assert(sizeof(UITouch*) == sizeof(int));

void ltIOSInit() {
    ltInitGraphics();
    const char *path = ltIOSBundlePath("main", ".lua");
    ltLuaSetup(path);
    delete[] path;
}

void ltIOSTeardown() {
    ltLuaTeardown();
}

void ltIOSRender() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ltLuaRender();
    ltLuaAdvance();
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
