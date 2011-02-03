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

void ltIOSTouchesBegin(NSSet *touches) {
}
void ltLuaTouchesMove(NSSet *touches) {
}
void ltLuaTouchesEnd(NSSet *touches) {
}
