/* Copyright (C) 2011 Ian MacLarty */

#include "ltlutil.h"

// Returns a reference to the lua object on the top of the stack
// and pops that object.
int ltlMakeRef(lua_State *L) {
    return luaL_ref(L, LUA_REGISTRYINDEX);
}

// Pushes onto the stack the object associated with a reference
// previously created with ltlMakeRef.
void ltlGetRef(lua_State *L, int ref) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
}

// Removes a reference previously created with ltlMakeRef.
// The reference becomes invalid.
void ltlUnref(lua_State *L, int ref) {
    luaL_unref(L, LUA_REGISTRYINDEX, ref);
}
