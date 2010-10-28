#include "lt.h"

static const char *file;

static void Setup() {
    ltLuaSetup(file);
}

int main(int argc, const char **argv) {
    if (argc == 2) {
        file = argv[1];
    } else {
        file = "../tests/test.lua";
    }
    ltHarnessInit(false, "Test", 60, Setup, ltLuaTeardown,
        ltLuaRender, ltLuaAdvance, ltLuaKeyDown, ltLuaKeyUp, ltLuaMouseDown, ltLuaMouseUp, ltLuaMouseMove,
        ltLuaResizeWindow);
    return 0;
}
