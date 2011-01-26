/* Copyright (C) 2011 Ian MacLarty */

#ifndef LTLUTIL_H
#define LTLUTIL_H

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// Returns a reference to the lua object on the top of the stack
// and pops that object.
int ltlMakeRef(lua_State *L);

// Pushes onto the stack the object associated with a reference
// previously created with ltlMakeRef.
void ltlGetRef(lua_State *L, int ref);

// Removes a reference previously created with ltlMakeRef.
// The reference becomes invalid.
void ltlUnref(lua_State *L, int ref);

#endif
