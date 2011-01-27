/* Copyright (C) 2011 Ian MacLarty */

#include "ltgraphics.h"
#include "ltlevent.h"
#include "ltlutil.h"

static bool call_handler(lua_State *L, int func, LTfloat x, LTfloat y, int button) {
    ltlGetRef(L, func);
    if (lua_isfunction(L, -1)) {
        lua_pushnumber(L, x);
        lua_pushnumber(L, y);
        lua_pushinteger(L, button);
        lua_call(L, 3, 1);
        bool consumed = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return consumed;
    } else {
        return false;
    }
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

LTLPointerMoveEventHandler::LTLPointerMoveEventHandler(lua_State *L) {
    LTLPointerMoveEventHandler::L = L;
    lua_func_ref = ltlMakeRef(L);
}

LTLPointerMoveEventHandler::~LTLPointerMoveEventHandler() {
    ltlUnref(L, lua_func_ref);
}

bool LTLPointerMoveEventHandler::consume(LTfloat x, LTfloat y, LTProp *prop, LTPointerEvent *event) {
    if (event->type == LT_EVENT_POINTER_MOVE) {
        return call_handler(L, lua_func_ref, x, y, event->button);
    } else {
        return false;
    }
}

LTLPointerOverEventHandler::LTLPointerOverEventHandler(lua_State *L) {
    LTLPointerOverEventHandler::L = L;
    lua_exit_func_ref = ltlMakeRef(L);
    lua_enter_func_ref = ltlMakeRef(L);
    first_time = true;
    in = false;
}

LTLPointerOverEventHandler::~LTLPointerOverEventHandler() {
    ltlUnref(L, lua_enter_func_ref);
    ltlUnref(L, lua_exit_func_ref);
}

bool LTLPointerOverEventHandler::consume(LTfloat x, LTfloat y, LTProp *prop, LTPointerEvent *event) {
    if (event->type == LT_EVENT_POINTER_MOVE) {
        bool containsPoint = prop->containsPoint(x, y);
        if (first_time) {
            first_time = false;
            in = containsPoint;
            if (in) {
                return call_handler(L, lua_enter_func_ref, x, y, event->button);
            } else {
                return false;
            }
        } else {
            bool res = false;
            if (containsPoint && !in) {
                res = call_handler(L, lua_enter_func_ref, x, y, event->button);
            } else if (!containsPoint && in) {
                res = call_handler(L, lua_exit_func_ref, x, y, event->button);
            }
            in = containsPoint;
            return res;
        }
    } else {
        return false;
    }
}
