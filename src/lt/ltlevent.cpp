/* Copyright (C) 2011 Ian MacLarty */

#include "ltgraphics.h"
#include "ltlevent.h"
#include "ltlutil.h"

static bool call_handler(lua_State *L, int func, LTfloat x, LTfloat y, int button) {
    ltlGetRef(L, func);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushinteger(L, button);
    lua_call(L, 3, 1);
    bool consumed = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return consumed;
}

LTLPointerDownInEventHandler::LTLPointerDownInEventHandler(lua_State *L) {
    LTLPointerDownInEventHandler::L = L;
    lua_func_ref = ltlMakeRef(L);
}

LTLPointerDownInEventHandler::~LTLPointerDownInEventHandler() {
    ltlUnref(L, lua_func_ref);
}

bool LTLPointerDownInEventHandler::consume(LTfloat x, LTfloat y, LTProp *prop, LTPointerEvent *event) {
    if (event->type == LT_EVENT_POINTER_DOWN) {
        if (prop->containsPoint(x, y)) {
            return call_handler(L, lua_func_ref, x, y, event->button);
        } else {
            return false;
        }
    } else {
        return false;
    }
}
