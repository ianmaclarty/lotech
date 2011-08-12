#include "lt.h"

static const char *file;

static void Setup() {
    #ifdef LTDEVMODE
    ltClientInit();
    #endif

    ltLuaSetup();
}

int main(int argc, const char **argv) {
    if (argc == 2) {
        file = argv[1];
    } else {
        file = "main.lua";
    }
    ltHarnessInit(false, "Test", 60, Setup, ltLuaTeardown,
        ltLuaRender, ltLuaAdvance, ltLuaKeyDown, ltLuaKeyUp, ltLuaPointerDown, ltLuaPointerUp, ltLuaPointerMove,
        ltLuaResizeWindow);
    return 0;
}
