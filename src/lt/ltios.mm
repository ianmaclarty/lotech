#include "ltlua.h"
#include "ltiosutil.h"

void ltIOSInit(const char *file) {
    const char *path = ltIOSBundlePath(file);
    ltLuaSetup(path);
    delete[] path;
}

void ltIOSTeardown() {
    ltLuaTeardown();
}

void ltIOSRender() {
    ltLuaAdvance();
    ltLuaRender();
}

void ltIOSTouchesBegan(NSSet *touches) {
}
void ltIOSTouchesMoved(NSSet *touches) {
}
void ltIOSTouchesEnded(NSSet *touches) {
}
