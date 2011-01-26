/* Copyright (C) 2011 Ian MacLarty */

#ifndef LTLEVENT_H
#define LTLEVENT_H

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "ltcommon.h"
#include "ltevent.h"

struct LTLPointerDownInEventHandler : LTPointerEventHandler {
    lua_State *L;
    int lua_func_ref;

    // Pops the lua function on the top of the stack and uses it
    // as the event handler.
    LTLPointerDownInEventHandler(lua_State *L);
    virtual ~LTLPointerDownInEventHandler();

    virtual bool consume(LTfloat x, LTfloat y, LTProp *prop, LTPointerEvent *event);
};

#endif
