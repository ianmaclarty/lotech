/* Copyright (C) 2010, 2011 Ian MacLarty */

#include "lt.h"

#define LT_USERDATA_MT "ltud"
#define LT_USERDATA_KEY "_ud"

static lua_State *g_L = NULL;
static int g_userdata_key_ref = LUA_NOREF;
static int g_wrefs_ref = LUA_NOREF;
static int g_string_table_ref = LUA_NOREF;
static int g_ud_metatables_ref = LUA_NOREF;
static bool g_suspended = false;
static bool g_initialized = false;
static bool g_gamecenter_initialized = false;

/************************* Functions for calling lua **************************/

// Check lua_pcall return status.
static void check_status(int status) {
    if (status) {
        const char *msg = lua_tostring(g_L, -1);
        lua_pop(g_L, 1);
        if (msg == NULL) msg = "Unknown error (error object is not a string).";
        ltLog(msg);
        #ifdef LTDEVMODE
        ltLog("Execution suspended");
        g_suspended = true;
        #else
        // TODO Notify the user of the error and offer them the option of emailing a report.
        ltAbort();
        #endif
    }
}

// Copied from lua source.
#ifdef LTDEVMODE
static int traceback(lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}
#endif

// Copied from lua source and modified.
static void docall(lua_State *L, int nargs) {
  int status;
#ifdef LTDEVMODE
  int base = lua_gettop(L) - nargs;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  status = lua_pcall(L, nargs, 0, base);
  lua_remove(L, base);  /* remove traceback function */
#else
  status = lua_pcall(L, nargs, 0, 0);
#endif
  check_status(status);
}

/************************* Weak references **************************/

// Returns a weak reference to the value at the given index.  Does not
// modify the stack.
static int make_weak_ref(lua_State *L, int index) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_wrefs_ref);
    if (index > 0) {
        lua_pushvalue(L, index);
    } else {
        lua_pushvalue(L, index - 2);
    }
    int ref = luaL_ref(L, -2);
    lua_pop(L, 1); // pop wrefs.
    return ref;
}

// Pushes referenced value.
static void get_weak_ref(lua_State *L, int ref) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_wrefs_ref);
    lua_rawgeti(L, -1, ref);
    lua_remove(L, -2); // remove wrefs.
}

/************************* Wrapping/unwrapping of c++ objects ******/

// Extract LTObject from wrapper table at the given index.
// Does not modify stack.
static LTObject* get_object(lua_State *L, int index, LTType type) {
    if (!lua_istable(L, index)) {
        luaL_error(L, "Expecting a table in argument %d.", index);
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_userdata_key_ref);
    if (index < 0) index--;
    lua_rawget(L, index);
    LTObject **ud = (LTObject**)luaL_checkudata(L, -1, LT_USERDATA_MT);
    lua_pop(L, 1);
    if (index < 0) index++;
    if (ud == NULL) {
        luaL_error(L, "ud == NULL.");
    }
    LTObject *o = *ud;
    if (o == NULL) {
        luaL_error(L, "o == NULL.");
    }
    if (!o->hasType(type)) {
        luaL_typerror(L, index, ltTypeName(type));
    }
    return o;
}

static int lt_SetObjectField(lua_State *L) {
    const char *fname;
    LTObject *obj = get_object(L, 1, LT_TYPE_OBJECT);
    fname = luaL_checkstring(L, 2);
    LTFieldDescriptor *field = obj->field(fname);
    if (field != NULL) {
        switch (field->type) {
            case LT_FIELD_TYPE_FLOAT: {
                LTfloat val = (LTfloat)luaL_checknumber(L, 3);
                obj->setFloatField(field, val);
                break;
            }
            case LT_FIELD_TYPE_INT: {
                LTint val = (LTint)luaL_checkinteger(L, 3);
                obj->setIntField(field, val);
                break;
            }
            case LT_FIELD_TYPE_BOOL: {
                LTbool val = (LTbool)lua_toboolean(L, 3);
                obj->setBoolField(field, val);
                break;
            }
        }
    }
    return 0;
}
    
static int lt_GetObjectField(lua_State *L) {
    LTObject *obj = get_object(L, 1, LT_TYPE_OBJECT);
    const char *fname = luaL_checkstring(L, 2);
    LTFieldDescriptor *field = obj->field(fname);
    if (field != NULL) {
        switch (field->type) {
            case LT_FIELD_TYPE_FLOAT: {
                lua_pushnumber(L, obj->getFloatField(field));
                break;
            }
            case LT_FIELD_TYPE_INT: {
                lua_pushinteger(L, obj->getIntField(field));
                break;
            }
            case LT_FIELD_TYPE_BOOL: {
                lua_pushboolean(L, obj->getBoolField(field));
                break;
            }
        }
        return 1;
    } else {
        lua_pushnil(L);
        return 1;
    }
}

static int delete_object(lua_State *L) {
    LTObject **ud = (LTObject**)lua_touserdata(L, 1);
    delete (*ud);
    *ud = NULL;
    return 0;
}

// Pushes the wrapper table for the given object.
// If the object has no wrapper table yet, then a new table
// is created.
static void push_wrap(lua_State *L, LTObject *obj) {
    if (obj->lua_wrap != LUA_NOREF) {
        // The object already has a wrapper, so push it and return.
        get_weak_ref(L, obj->lua_wrap);
        return;
    }
    lua_newtable(L);
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_ud_metatables_ref);
    lua_getfield(L, -1, ltTypeName(obj->type));
    lua_setmetatable(L, -3);
    lua_pop(L, 1); // pop metatables. wrapper table now on top.
    // Push wrapper table field that will point to the C++ object.
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_userdata_key_ref);
    // Push user data for C++ obj.
    LTObject **ud = (LTObject **)lua_newuserdata(L, sizeof(LTObject *));
    *ud = obj;
    // Add metatable for userdata with gc finalizer.
    if (luaL_newmetatable(L, LT_USERDATA_MT)) {
        lua_pushcfunction(L, delete_object);
        lua_setfield(L, -2, "__gc");
    }
    lua_setmetatable(L, -2);
    lua_rawset(L, -3);
    // Wrapper table should now be on the top of the stack.
    obj->lua_wrap = make_weak_ref(L, -1);
}

// Inserts the object at the given index into the wrapper
// table at the given index so that the GC can trace it.
// If the object is already in the table, its reference count is
// incremented.
static void add_ref(lua_State *L, int wrap_index, int ref_index) {
    lua_pushvalue(L, ref_index);
    if (wrap_index > 0) {
        lua_rawget(L, wrap_index);
    } else {
        lua_rawget(L, wrap_index - 1);
    }
    int val = 1;
    if (!lua_isnil(L, -1)) {
        val = lua_tointeger(L, -1) + 1;
    }
    lua_pop(L, 1);
    lua_pushvalue(L, ref_index);
    lua_pushinteger(L, val);
    if (wrap_index > 0) {
        lua_rawset(L, wrap_index);
    } else {
        lua_rawset(L, wrap_index - 2);
    }
}

// Removes the object at the given index from the wrapper
// table if its reference count is 1, otherwise decrements
// the reference count.
static void del_ref(lua_State *L, int wrap_index, int ref_index) {
    lua_pushvalue(L, ref_index);
    if (wrap_index > 0) {
        lua_rawget(L, wrap_index);
    } else {
        lua_rawget(L, wrap_index - 1);
    }
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }
    int val = lua_tointeger(L, -1) - 1;
    lua_pop(L, 1);
    lua_pushvalue(L, ref_index);
    if (val <= 0) {
        lua_pushnil(L);
    } else {
        lua_pushinteger(L, val);
    }
    if (wrap_index > 0) {
        lua_rawset(L, wrap_index);
    } else {
        lua_rawset(L, wrap_index - 2);
    }
}

// Set a field of a wrapper table to point to a reference.
// Sometimes we use this instead of add_ref,
// so the other object can be more easily accessed from lua code.
static void set_ref_field(lua_State *L, int wrap_index, const char *field, int ref_index) {
    lua_pushstring(L, field);
    lua_pushvalue(L, ref_index);
    if (wrap_index > 0) {
        lua_rawset(L, wrap_index);
    } else {
        lua_rawset(L, wrap_index - 2);
    }
}

static void set_object_fields_from_table(lua_State *L, LTObject *obj, int table) {
    lua_pushnil(L);
    if (table < 0) {
        table--;
    }
    while (lua_next(L, table) != 0) {
        int key_type = lua_type(L, -2);
        if (key_type == LUA_TSTRING) {
            const char *key = lua_tostring(L, -2);
            LTFieldDescriptor *field = obj->field(key);
            if (field != NULL) {
                switch (field->type) {
                    case LT_FIELD_TYPE_FLOAT: {
                        obj->setFloatField(field, lua_tonumber(L, -1));
                        break;
                    }
                    case LT_FIELD_TYPE_INT: {
                        obj->setIntField(field, lua_tointeger(L, -1));
                        break;
                    }
                    case LT_FIELD_TYPE_BOOL: {
                        obj->setBoolField(field, lua_toboolean(L, -1));
                        break;
                    }
                }
            }
        }
        lua_pop(L, 1);
    }
}

/************************** General utility functions ************/

static int check_nargs(lua_State *L, int exp_args) {
    int n = lua_gettop(L);
    if (n < exp_args) {
        return luaL_error(L, "Expecting at least %d args, got %d.", exp_args, n);
    } else {
        return n;
    }
}

static int max_atlas_size() {
    #ifdef LTIOS
        if (ltIsIPad() || ltIsRetinaIPhone()) {
            return 2048;
        } else {
            return 1024;
        }
    #else
        return 2048;
    #endif
}

/************************** Resolve paths ****************/

static const char* resource_prefix = "";

static const char *resource_path(const char *resource, const char *suffix) {
    const char *path;
    #ifdef LTIOS
        path = ltIOSBundlePath(resource, suffix);
    #elif LTOSX
        path = ltOSXBundlePath(resource, suffix);
    #else
        int len = strlen(resource_prefix) + strlen(resource) + strlen(suffix) + 3;
        path = new char[len];
        snprintf((char*)path, len, "%s%s%s", resource_prefix, resource, suffix);
    #endif
    return path;
}

static const char *image_path(const char *name) {
    #ifdef LTIOS
        const char *path;
        if (ltIsIPad() || ltIsRetinaIPhone()) {
            path = ltIOSBundlePath(name, ".png2x");
            if (ltFileExists(path)) {
                return path;
            }
            delete[] path;
        }
        path = ltIOSBundlePath(name, ".png1x");
        if (ltFileExists(path)) {
            return path;
        } else {
            delete[] path;
            return ltIOSBundlePath(name, ".png");
        }
    #else
        return resource_path(name, ".png");
    #endif
}

static const char *sound_path(const char *name) {
    return resource_path(name, ".wav");
}

/************************* Graphics **************************/

static int lt_SetViewPort(lua_State *L) {
    check_nargs(L, 4);
    LTfloat viewport_x1 = (LTfloat)luaL_checknumber(L, 1);
    LTfloat viewport_y1 = (LTfloat)luaL_checknumber(L, 2);
    LTfloat viewport_x2 = (LTfloat)luaL_checknumber(L, 3);
    LTfloat viewport_y2 = (LTfloat)luaL_checknumber(L, 4);
    ltSetViewPort(viewport_x1, viewport_y1, viewport_x2, viewport_y2);
    return 0;
}

static int lt_SetDesignScreenSize(lua_State *L) {
    check_nargs(L, 2);
    LTfloat w = (LTfloat)luaL_checknumber(L, 1);
    LTfloat h = (LTfloat)luaL_checknumber(L, 2);
    ltSetDesignScreenSize(w, h);
    return 0;
}

static int lt_SetOrientation(lua_State *L) {
    check_nargs(L, 1);
    const char *orientation_str = lua_tostring(L, 1);
    LTDisplayOrientation orientation;
    if (strcmp(orientation_str, "portrait") == 0) {
        orientation = LT_DISPLAY_ORIENTATION_PORTRAIT;
    } else if (strcmp(orientation_str, "landscape") == 0) {
        orientation = LT_DISPLAY_ORIENTATION_LANDSCAPE;
    } else {
        return luaL_error(L, "Invalid orientation: %s", orientation_str);
    }
    ltSetDisplayOrientation(orientation);
    return 0;
}

static int lt_PushTint(lua_State *L) {
    int num_args = check_nargs(L, 3);
    LTfloat r = (LTfloat)luaL_checknumber(L, 1);
    LTfloat g = (LTfloat)luaL_checknumber(L, 2);
    LTfloat b = (LTfloat)luaL_checknumber(L, 3);
    LTfloat a;
    if (num_args > 3) {
        a = (LTfloat)luaL_checknumber(L, 4);
    } else {
        a = 1.0f;
    }
    ltPushTint(r, g, b, a);
    return 0;
}

static int lt_PopTint(lua_State *L) {
    ltPopTint();
    return 0;
}

static int lt_PushMatrix(lua_State *L) {
    ltPushMatrix();
    return 0;
}

static int lt_PopMatrix(lua_State *L) {
    ltPopMatrix();
    return 0;
}

static int lt_DrawUnitSquare(lua_State *L) {
    ltDrawUnitSquare();
    return 0;
}

static int lt_DrawUnitCircle(lua_State *L) {
    ltDrawUnitCircle();
    return 0;
}

static int lt_DrawRect(lua_State *L) {
    check_nargs(L, 4);
    LTfloat x1 = (LTfloat)luaL_checknumber(L, 1);
    LTfloat y1 = (LTfloat)luaL_checknumber(L, 2);
    LTfloat x2 = (LTfloat)luaL_checknumber(L, 3);
    LTfloat y2 = (LTfloat)luaL_checknumber(L, 4);
    ltDrawRect(x1, y1, x2, y2);
    return 0;
}

static int lt_DrawEllipse(lua_State *L) {
    check_nargs(L, 4);
    LTfloat x = (LTfloat)luaL_checknumber(L, 1);
    LTfloat y = (LTfloat)luaL_checknumber(L, 2);
    LTfloat rx = (LTfloat)luaL_checknumber(L, 3);
    LTfloat ry = (LTfloat)luaL_checknumber(L, 4);
    ltDrawEllipse(x, y, rx, ry);
    return 0;
}

static int lt_DrawSceneNode(lua_State *L) {
    int nargs = check_nargs(L, 1);
    LTSceneNode *node = (LTSceneNode*)get_object(L, 1, LT_TYPE_SCENENODE);
    if (nargs > 1) {
        LTRenderTarget *target = (LTRenderTarget *)get_object(L, 2, LT_TYPE_RENDERTARGET);
        LTColor clear_color(0.0f, 0.0f, 0.0f, 0.0f);
        bool do_clear = false;
        if (nargs > 2) {
            do_clear = true;
            clear_color.r = luaL_checknumber(L, 3);
        }
        if (nargs > 3) {
            clear_color.g = luaL_checknumber(L, 4);
        }
        if (nargs > 4) {
            clear_color.b = luaL_checknumber(L, 5);
        }
        if (nargs > 5) {
            clear_color.a = luaL_checknumber(L, 6);
        }
        target->renderNode(node, do_clear ? &clear_color : NULL);
    } else {
        node->draw();
    }
    return 0;
}

static int lt_InsertLayerFront(lua_State *L) {
    int num_args = check_nargs(L, 2);
    LTLayer *layer = (LTLayer*)get_object(L, 1, LT_TYPE_LAYER);
    LTSceneNode *node = (LTSceneNode*)get_object(L, 2, LT_TYPE_SCENENODE);
    if (num_args > 2) {
        luaL_error(L, "Only two arguments expected");
    }
    layer->insert_front(node);
    add_ref(L, 1, 2);
    return 0;
}

static int lt_InsertLayerBack(lua_State *L) {
    int num_args = check_nargs(L, 2);
    LTLayer *layer = (LTLayer*)get_object(L, 1, LT_TYPE_LAYER);
    LTSceneNode *node = (LTSceneNode*)get_object(L, 2, LT_TYPE_SCENENODE);
    if (num_args > 2) {
        luaL_error(L, "Only two arguments expected");
    }
    layer->insert_back(node);
    add_ref(L, 1, 2);
    return 0;
}

static int lt_InsertLayerAbove(lua_State *L) {
    int num_args = check_nargs(L, 3);
    LTLayer *layer = (LTLayer*)get_object(L, 1, LT_TYPE_LAYER);
    LTSceneNode *existing_node = (LTSceneNode*)get_object(L, 2, LT_TYPE_SCENENODE);
    LTSceneNode *new_node = (LTSceneNode*)get_object(L, 3, LT_TYPE_SCENENODE);
    if (num_args > 3) {
        luaL_error(L, "Only three arguments expected");
    }
    if (layer->insert_above(existing_node, new_node)) {
        add_ref(L, 1, 3);
    }
    return 0;
}

static int lt_InsertLayerBelow(lua_State *L) {
    int num_args = check_nargs(L, 3);
    LTLayer *layer = (LTLayer*)get_object(L, 1, LT_TYPE_LAYER);
    LTSceneNode *existing_node = (LTSceneNode*)get_object(L, 2, LT_TYPE_SCENENODE);
    LTSceneNode *new_node = (LTSceneNode*)get_object(L, 3, LT_TYPE_SCENENODE);
    if (num_args > 3) {
        luaL_error(L, "Only three arguments expected");
    }
    if (layer->insert_below(existing_node, new_node)) {
        add_ref(L, 1, 3);
    }
    return 0;
}

static int lt_LayerSize(lua_State *L) {
    check_nargs(L, 1);
    LTLayer *layer = (LTLayer*)get_object(L, 1, LT_TYPE_LAYER);
    lua_pushinteger(L, layer->size());
    return 1;
}

static int lt_RemoveFromLayer(lua_State *L) {
    check_nargs(L, 2);
    LTLayer *layer = (LTLayer*)get_object(L, 1, LT_TYPE_LAYER);
    LTSceneNode *node = (LTSceneNode*)get_object(L, 2, LT_TYPE_SCENENODE);
    layer->remove(node);
    del_ref(L, 1, 2);
    return 0;
}

static int lt_Layer(lua_State *L) {
    int num_args = check_nargs(L, 0);
    LTLayer *layer = new LTLayer();
    push_wrap(L, layer);
    // Add arguments as child nodes.
    // First arguments are drawn in front of last arguments.
    for (int arg = 1; arg <= num_args; arg++) {
        LTSceneNode *child = (LTSceneNode*)get_object(L, arg, LT_TYPE_SCENENODE);
        layer->insert_back(child);
        add_ref(L, -1, arg); // Add reference from layer node to child node.
    }
    return 1;
}

static int lt_Translate(lua_State *L) {
    int num_args = check_nargs(L, 3);
    LTSceneNode *child = (LTSceneNode *)get_object(L, 1, LT_TYPE_SCENENODE);
    LTfloat x = (LTfloat)luaL_checknumber(L, 2);
    LTfloat y = (LTfloat)luaL_checknumber(L, 3);
    LTfloat z = 0.0f;
    if (num_args > 3) {
        z = (LTfloat)luaL_checknumber(L, 4);
    }
    LTTranslateNode *node = new LTTranslateNode(x, y, z, child);
    push_wrap(L, node);
    set_ref_field(L, -1, "child", 1); // Add reference from new node to child.
    return 1;
}

static int lt_Rotate(lua_State *L) {
    int nargs = check_nargs(L, 2);
    LTSceneNode *child = (LTSceneNode *)get_object(L, 1, LT_TYPE_SCENENODE);
    LTdegrees angle = (LTfloat)luaL_checknumber(L, 2);
    LTfloat cx = 0.0f;
    LTfloat cy = 0.0f;
    if (nargs > 2) {
        cx = luaL_checknumber(L, 3);
    }
    if (nargs > 3) {
        cy = luaL_checknumber(L, 4);
    }
    LTRotateNode *node = new LTRotateNode(angle, cx, cy, child);
    push_wrap(L, node);
    set_ref_field(L, -1, "child", 1); // Add reference from new node to child.
    return 1;
}

static int lt_Scale(lua_State *L) {
    int num_args = check_nargs(L, 2);
    LTSceneNode *child = (LTSceneNode *)get_object(L, 1, LT_TYPE_SCENENODE);
    LTfloat sx = 1.0f;
    LTfloat sy = 1.0f;
    LTfloat sz = 1.0f;
    LTfloat s = 1.0f;
    if (num_args == 2) {
        s = luaL_checknumber(L, 2);
    } else {
        sx = luaL_checknumber(L, 2);
        sy = luaL_checknumber(L, 3);
    }
    if (num_args > 3) {
        sz = luaL_checknumber(L, 4);
    }
    LTScaleNode *node = new LTScaleNode(sx, sy, sz, s, child);
    push_wrap(L, node);
    set_ref_field(L, -1, "child", 1); // Add reference from new node to child.
    return 1;
}

static int lt_Perspective(lua_State *L) {
    int nargs = check_nargs(L, 4);
    LTSceneNode *child = (LTSceneNode *)get_object(L, 1, LT_TYPE_SCENENODE);
    LTfloat near = luaL_checknumber(L, 2);
    LTfloat origin = luaL_checknumber(L, 3);
    LTfloat far = luaL_checknumber(L, 4);
    LTfloat vanish_x = 0.0f;
    LTfloat vanish_y = 0.0f;
    if (nargs > 4) {
        vanish_x = luaL_checknumber(L, 5);
    }
    if (nargs > 5) {
        vanish_y = luaL_checknumber(L, 6);
    }
    LTPerspective *node = new LTPerspective(near, origin, far, vanish_x, vanish_y, child);
    push_wrap(L, node);
    set_ref_field(L, -1, "child", 1); // Add reference from new node to child.
    return 1;
}

static int lt_Pitch(lua_State *L) {
    check_nargs(L, 2);
    LTSceneNode *child = (LTSceneNode *)get_object(L, 1, LT_TYPE_SCENENODE);
    LTfloat pitch = (LTfloat)luaL_checknumber(L, 2);
    LTPitch *node = new LTPitch(pitch, child);
    push_wrap(L, node);
    set_ref_field(L, -1, "child", 1); // Add reference from new node to child.
    return 1;
}

static int lt_Fog(lua_State *L) {
    check_nargs(L, 6);
    LTSceneNode *child = (LTSceneNode *)get_object(L, 1, LT_TYPE_SCENENODE);
    LTfloat start = (LTfloat)luaL_checknumber(L, 2);
    LTfloat end = (LTfloat)luaL_checknumber(L, 3);
    LTfloat red = (LTfloat)luaL_checknumber(L, 4);
    LTfloat green = (LTfloat)luaL_checknumber(L, 5);
    LTfloat blue = (LTfloat)luaL_checknumber(L, 6);
    LTColor color(red, green, blue, 1.0f); // Alpha ignored for fog.
    LTFog *fog = new LTFog(start, end, color, child);
    push_wrap(L, fog);
    set_ref_field(L, -1, "child", 1); // Add ref from new node to child.
    return 1;
}

static int lt_DepthTest(lua_State *L) {
    int nargs = check_nargs(L, 1);
    LTSceneNode *child = (LTSceneNode *)get_object(L, 1, LT_TYPE_SCENENODE);
    bool on = true;
    if (nargs > 1) {
        on = lua_toboolean(L, 2);
    }
    push_wrap(L, new LTDepthTest(on, child));
    set_ref_field(L, -1, "child", 1); // Add ref from new node to child.
    return 1;
}

static int lt_DepthMask(lua_State *L) {
    int nargs = check_nargs(L, 1);
    LTSceneNode *child = (LTSceneNode *)get_object(L, 1, LT_TYPE_SCENENODE);
    bool on = true;
    if (nargs > 1) {
        on = lua_toboolean(L, 2);
    }
    push_wrap(L, new LTDepthMask(on, child));
    set_ref_field(L, -1, "child", 1); // Add ref from new node to child.
    return 1;
}

static int lt_Tint(lua_State *L) {
    int num_args = check_nargs(L, 4);
    LTSceneNode *child = (LTSceneNode *)get_object(L, 1, LT_TYPE_SCENENODE);
    LTfloat r = (LTfloat)luaL_checknumber(L, 2);
    LTfloat g = (LTfloat)luaL_checknumber(L, 3);
    LTfloat b = (LTfloat)luaL_checknumber(L, 4);
    LTfloat a = 1.0f;
    if (num_args > 4) {
        a = (LTfloat)luaL_checknumber(L, 5);
    }
    LTTintNode *tinter = new LTTintNode(r, g, b, a, child);
    push_wrap(L, tinter);
    set_ref_field(L, -1, "child", 1); // Add reference from new node to child.
    return 1;
}

static int lt_BlendMode(lua_State *L) {
    check_nargs(L, 2);
    LTSceneNode *child = (LTSceneNode *)get_object(L, 1, LT_TYPE_SCENENODE);
    const char *modestr = lua_tostring(L, 2);
    if (modestr == NULL) {
        return luaL_error(L, "Expecting a string in argument 2");
    }
    LTBlendMode mode;
    if (strcmp(modestr, "add") == 0) {
        mode = LT_BLEND_MODE_ADD;
    } else if (strcmp(modestr, "subtract") == 0) {
        mode = LT_BLEND_MODE_SUBTRACT;
    //} else if (strcmp(modestr, "diff") == 0) {
    //    mode = LT_BLEND_MODE_DIFF;
    } else if (strcmp(modestr, "color") == 0) {
        mode = LT_BLEND_MODE_COLOR;
    } else if (strcmp(modestr, "multiply") == 0) {
        mode = LT_BLEND_MODE_MULTIPLY;
    } else if (strcmp(modestr, "normal") == 0) {
        mode = LT_BLEND_MODE_NORMAL;
    } else if (strcmp(modestr, "off") == 0) {
        mode = LT_BLEND_MODE_OFF;
    } else {
        return luaL_error(L, "Invalid blend mode: %s", modestr);
    }
    LTBlendModeNode *blend = new LTBlendModeNode(mode, child);
    push_wrap(L, blend);
    set_ref_field(L, -1, "child", 1); // Add reference from new node to child.
    return 1;
}

static int lt_TextureMode(lua_State *L) {
    check_nargs(L, 2);
    LTSceneNode *child = (LTSceneNode *)get_object(L, 1, LT_TYPE_SCENENODE);
    const char *modestr = lua_tostring(L, 2);
    if (modestr == NULL) {
        return luaL_error(L, "Expecting a string in argument 2");
    }
    LTTextureMode mode;
    if (strcmp(modestr, "add") == 0) {
        mode = LT_TEXTURE_MODE_ADD;
    } else if (strcmp(modestr, "decal") == 0) {
        mode = LT_TEXTURE_MODE_DECAL;
    } else if (strcmp(modestr, "blend") == 0) {
        mode = LT_TEXTURE_MODE_BLEND;
    } else if (strcmp(modestr, "replace") == 0) {
        mode = LT_TEXTURE_MODE_REPLACE;
    } else if (strcmp(modestr, "modulate") == 0) {
        mode = LT_TEXTURE_MODE_MODULATE;
    } else {
        return luaL_error(L, "Invalid texture mode: %s", modestr);
    }
    LTTextureModeNode *node = new LTTextureModeNode(mode, child);
    push_wrap(L, node);
    set_ref_field(L, -1, "child", 1); // Add reference from new node to child.
    return 1;
}

static int lt_Line(lua_State *L) {
    check_nargs(L, 4);
    LTfloat x1 = (LTfloat)luaL_checknumber(L, 1);
    LTfloat y1 = (LTfloat)luaL_checknumber(L, 2);
    LTfloat x2 = (LTfloat)luaL_checknumber(L, 3);
    LTfloat y2 = (LTfloat)luaL_checknumber(L, 4);
    LTLineNode *line = new LTLineNode(x1, y1, x2, y2);
    push_wrap(L, line);
    return 1;
}

static int lt_Triangle(lua_State *L) {
    check_nargs(L, 6);
    LTfloat x1 = (LTfloat)luaL_checknumber(L, 1);
    LTfloat y1 = (LTfloat)luaL_checknumber(L, 2);
    LTfloat x2 = (LTfloat)luaL_checknumber(L, 3);
    LTfloat y2 = (LTfloat)luaL_checknumber(L, 4);
    LTfloat x3 = (LTfloat)luaL_checknumber(L, 5);
    LTfloat y3 = (LTfloat)luaL_checknumber(L, 6);
    LTTriangleNode *triangle = new LTTriangleNode(x1, y1, x2, y2, x3, y3);
    push_wrap(L, triangle);
    return 1;
}

static int lt_Rect(lua_State *L) {
    check_nargs(L, 4);
    LTfloat x1 = (LTfloat)luaL_checknumber(L, 1);
    LTfloat y1 = (LTfloat)luaL_checknumber(L, 2);
    LTfloat x2 = (LTfloat)luaL_checknumber(L, 3);
    LTfloat y2 = (LTfloat)luaL_checknumber(L, 4);
    LTRectNode *node = new LTRectNode(x1, y1, x2, y2);
    push_wrap(L, node);
    return 1;
}

static int lt_HitFilter(lua_State *L) {
    check_nargs(L, 5);
    LTSceneNode *child = (LTSceneNode *)get_object(L, 1, LT_TYPE_SCENENODE);
    LTfloat left = (LTfloat)luaL_checknumber(L, 2);
    LTfloat bottom = (LTfloat)luaL_checknumber(L, 3);
    LTfloat right = (LTfloat)luaL_checknumber(L, 4);
    LTfloat top = (LTfloat)luaL_checknumber(L, 5);
    LTHitFilter *filter = new LTHitFilter(left, bottom, right, top, child);
    push_wrap(L, filter);
    set_ref_field(L, -1, "child", 1); // Add reference from new node to child.
    return 1;
}

static int lt_DownFilter(lua_State *L) {
    check_nargs(L, 5);
    LTSceneNode *child = (LTSceneNode *)get_object(L, 1, LT_TYPE_SCENENODE);
    LTfloat left = (LTfloat)luaL_checknumber(L, 2);
    LTfloat bottom = (LTfloat)luaL_checknumber(L, 3);
    LTfloat right = (LTfloat)luaL_checknumber(L, 4);
    LTfloat top = (LTfloat)luaL_checknumber(L, 5);
    LTDownFilter *filter = new LTDownFilter(left, bottom, right, top, child);
    push_wrap(L, filter);
    set_ref_field(L, -1, "child", 1); // Add reference from new node to child.
    return 1;
}

static int lt_Wrap(lua_State *L) {
    check_nargs(L, 1);
    LTSceneNode *child = (LTSceneNode *)get_object(L, 1, LT_TYPE_SCENENODE);
    LTWrapNode *wrap = new LTWrapNode(child);
    push_wrap(L, wrap);
    set_ref_field(L, -1, "child", 1); // Add reference from wrapper to child.
    return 1;
}

static int lt_ReplaceWrappedChild(lua_State *L) {
    check_nargs(L, 2);
    LTWrapNode *wrap = (LTWrapNode *)get_object(L, 1, LT_TYPE_WRAP);
    LTSceneNode *new_child = (LTSceneNode *)get_object(L, 2, LT_TYPE_SCENENODE);
    wrap->child = new_child;
    set_ref_field(L, 1, "child", 2); // Replace the child field with the new child.
    return 1;
}

/************************* Particle System ******************/

static int lt_ParticleSystem(lua_State *L) {
    check_nargs(L, 3);
    LTImage *img = (LTImage *)get_object(L, 1, LT_TYPE_IMAGE);
    int n = luaL_checkinteger(L, 2);
    LTParticleSystem *particles = new LTParticleSystem(img, n);
    if (!lua_istable(L, 3)) {
        return luaL_error(L, "Expecting a table in argument 3");
    }
    set_object_fields_from_table(L, particles, 3);
    if (particles->emission_rate < 0.0f) {
        particles->emission_rate = (LTfloat)particles->max_particles / particles->life;
    }
    push_wrap(L, particles);
    add_ref(L, -1, 1); // Add reference from particles to image.
    return 1;
}

static int lt_ParticleSystemAdvance(lua_State *L) {
    check_nargs(L, 2);
    LTParticleSystem *particles = (LTParticleSystem *)get_object(L, 1, LT_TYPE_PARTICLESYSTEM);
    LTfloat dt = luaL_checknumber(L, 2);
    particles->advance(dt);
    return 0;
}

static int lt_ParticleSystemFixtureFilter(lua_State *L) {
    check_nargs(L, 2);
    LTParticleSystem *particles = (LTParticleSystem *)get_object(L, 1, LT_TYPE_PARTICLESYSTEM);
    LTFixture *fixture = (LTFixture *)get_object(L, 2, LT_TYPE_FIXTURE);
    if (particles->fixture != NULL) {
        push_wrap(L, particles->fixture);
        del_ref(L, 1, -1); // Delete existing reference.
        lua_pop(L, 1);
    }
    particles->fixture = fixture;
    add_ref(L, 1, 2); // Add reference from particle system to fixture.
    return 0;
}

/************************* Vectors **************************/

static int lt_Vector(lua_State *L) {
    int num_args = check_nargs(L, 1);
    int capacity;
    int stride;
    LTVector *vec;
    if (num_args == 1 && lua_istable(L, 1)) {
        capacity = lua_objlen(L, 1);
        if (capacity > 0) {
            // Get first row to compute stride.
            lua_rawgeti(L, 1, 1);
            if (!lua_istable(L, -1)) {
                return luaL_error(L, "Expecting an array of arrays");
            }
            if (!lua_isnil(L, -1)) {
                stride = lua_objlen(L, -1);
            } else {
                stride = 0;
            }
            lua_pop(L, 1);
        } else {
            stride = 0;
        }
        vec = new LTVector(capacity, stride);
        for (int i = 1; i <= capacity; i++) {
            lua_rawgeti(L, 1, i);
            for (int j = 1; j <= stride; j++) {
                lua_rawgeti(L, -1, j);
                if (!lua_isnumber(L, -1)) {
                    return luaL_error(L, "Expecting a number in row %d, column %d", i, j);
                }
                vec->data[(i - 1) * stride + (j - 1)] = lua_tonumber(L, -1);
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
        }
        vec->size = capacity;
    } else if (num_args == 2) {
        int capacity = luaL_checkinteger(L, 1);
        int stride = luaL_checkinteger(L, 2);
        vec = new LTVector(capacity, stride);
        vec->size = capacity;
    } else {
        return luaL_error(L, "Invalid arguments");
    }
    push_wrap(L, vec);

    /*
    for (int i = 0; i < capacity; i++) {
        for (int j = 0; j < stride; j++) {
            fprintf(stderr, "%10f ", vec->data[i * stride + j]);
        }
        fprintf(stderr, "\n");
    }
    */

    return 1;
}

static int lt_GenerateVectorColumn(lua_State *L) {
    int num_args = check_nargs(L, 3);
    LTVector *v = (LTVector *)get_object(L, 1, LT_TYPE_VECTOR);
    int col = luaL_checkinteger(L, 2);
    int stride = v->stride;
    LTfloat lo = luaL_checknumber(L, 3);
    LTfloat hi = lo;
    if (num_args > 3) {
        hi = luaL_checknumber(L, 4);
    }
    if (col > stride || col < 1) {
        return luaL_error(L, "Invalid column: %d", col);
    }
    LTfloat *ptr = v->data + col - 1;
    LTfloat *end = ptr + v->size * stride;
    if (lo == hi) {
        while (ptr != end) {
            *ptr = lo;
            ptr += stride;
        }
    } else {
        while (ptr != end) {
            *ptr = ltRandBetween(lo, hi);
            ptr += stride;
        }
    }
    return 0;
}

static int lt_FillVectorColumnsWithImageQuads(lua_State *L) {
    check_nargs(L, 5);
    LTVector *vector = (LTVector *)get_object(L, 1, LT_TYPE_VECTOR);
    int col = luaL_checkinteger(L, 2) - 1;
    LTTexturedNode *img = (LTTexturedNode *)get_object(L, 3, LT_TYPE_TEXTUREDNODE);
    LTVector *offsets = (LTVector *)get_object(L, 4, LT_TYPE_VECTOR);
    int offsets_col = luaL_checkinteger(L, 5) - 1;
    if (vector->size < 4) {
        return luaL_error(L, "Vector size must be at least 4");
    }
    if ((vector->size & 3) > 0) {
        return luaL_error(L, "Vector size must be divisible by 4");
    }
    if (offsets->size != (vector->size >> 2)) {
        return luaL_error(L, "Offsets vector must be a quarter of the size of the target vector");
    }
    if (vector->stride - col < 4) {
        return luaL_error(L, "Vector stride to small (must be at least 4)");
    }
    if (offsets->stride - offsets_col < 2) {
        return luaL_error(L, "Not enough columns in offsets vector (must be at least 2)");
    }
    int n = offsets->size;
    //fprintf(stderr, "n = %d, col = %d, offsets_col = %d, offsets = %p, vector = %p\n", n, col, offsets_col, offsets->data, vector->data);
    LTfloat *data = vector->data + col;
    LTfloat *os_data = offsets->data + offsets_col;
    for (int i = 0; i < n; i++) {
        data[0] = img->world_vertices[0] + os_data[0];
        data[1] = img->world_vertices[1] + os_data[1];
        data[2] = img->tex_coords[0];
        data[3] = img->tex_coords[1];
        data += vector->stride;
        data[0] = img->world_vertices[2] + os_data[0];
        data[1] = img->world_vertices[3] + os_data[1];
        data[2] = img->tex_coords[2];
        data[3] = img->tex_coords[3];
        data += vector->stride;
        data[0] = img->world_vertices[6] + os_data[0];
        data[1] = img->world_vertices[7] + os_data[1];
        data[2] = img->tex_coords[6];
        data[3] = img->tex_coords[7];
        data += vector->stride;
        data[0] = img->world_vertices[4] + os_data[0];
        data[1] = img->world_vertices[5] + os_data[1];
        data[2] = img->tex_coords[4];
        data[3] = img->tex_coords[5];
        data += vector->stride;
        os_data += offsets->stride;
    }
    return 0;
}

static int lt_DrawQuads(lua_State *L) {
    int num_args = check_nargs(L, 2);
    LTVector *vector = (LTVector *)get_object(L, 1, LT_TYPE_VECTOR);
    LTTexturedNode *img = (LTTexturedNode *)get_object(L, 2, LT_TYPE_TEXTUREDNODE);
    int col = 0;
    if (num_args > 2) {
        col = luaL_checkinteger(L, 3) - 1;
    }
    if (vector->stride - col < 4) {
        return luaL_error(L, "Not enough columns (need 4)");
    }
    LTDrawTexturedQuads *draw_quads = new LTDrawTexturedQuads(vector, col, img);
    push_wrap(L, draw_quads);
    add_ref(L, -1, 1); // Add reference from node to vector.
    add_ref(L, -1, 2); // Add reference from node to image.
    return 1;
}

static int lt_DrawVector(lua_State *L) {
    int num_args = check_nargs(L, 3);
    LTVector *vector = (LTVector *)get_object(L, 1, LT_TYPE_VECTOR);
    const char *mode_str = lua_tostring(L, 2);
    int dims = lua_tointeger(L, 3);
    if (dims != 2 && dims != 3) {
        return luaL_error(L, "Dimensions must be 2 or 3");
    }
    int color_os = -1;
    if (num_args > 3) {
        color_os = lua_tointeger(L, 4) - 1;
        if (color_os != -1 && color_os > (vector->stride - 4)) {
            return luaL_error(L, "Invalid color offset");
        }
    }
    int tex_os = -1;
    LTTexturedNode *img = NULL;
    if (num_args > 4) {
        tex_os = lua_tointeger(L, 5) - 1;
        if (tex_os != -1 && tex_os > (vector->stride - 2)) {
            return luaL_error(L, "Invalid texture offset");
        }
        // If there is a texture offset, an image should also be provided.
        if (tex_os != -1 && num_args < 6) {
            return luaL_error(L, "An image must be provided if a texture offset is given.");
        }
        img = (LTTexturedNode *)get_object(L, 6, LT_TYPE_TEXTUREDNODE);
    }
    LTDrawMode mode;
    if (strcmp(mode_str, "triangle_strip") == 0) {
        mode = LT_DRAWMODE_TRIANGLE_STRIP;
    } else if (strcmp(mode_str, "triangle_fan") == 0) {
        mode = LT_DRAWMODE_TRIANGLE_FAN;
    } else if (strcmp(mode_str, "triangles") == 0) {
        mode = LT_DRAWMODE_TRIANGLES;
    } else if (strcmp(mode_str, "lines") == 0) {
        mode = LT_DRAWMODE_LINES;
    } else if (strcmp(mode_str, "points") == 0) {
        mode = LT_DRAWMODE_POINTS;
    } else {
        return luaL_error(L, "Invalid draw mode");
    }

    LTDrawVector *draw_vec = new LTDrawVector(mode, vector, dims, 0, color_os, tex_os, img);
    push_wrap(L, draw_vec);
    add_ref(L, -1, 1); // Add reference from node to vector.
    if (img != NULL) {
        add_ref(L, -1, 5); // Add reference from node to image.
    }

    return 1;
}

/************************* Render targets ******************/

static int lt_RenderTarget(lua_State *L) {
    int nargs = check_nargs(L, 2);
    int w = luaL_checkinteger(L, 1);
    int h = luaL_checkinteger(L, 2);
    LTfloat vp_x1 = -1.0f;
    LTfloat vp_y1 = -1.0f;
    LTfloat vp_x2 = 1.0f;
    LTfloat vp_y2 = 1.0f;
    if (nargs > 2) {
        vp_x1 = luaL_checknumber(L, 3);
        vp_y1 = luaL_checknumber(L, 4);
        vp_x2 = luaL_checknumber(L, 5);
        vp_y2 = luaL_checknumber(L, 6);
    }
    LTfloat wld_x1;
    LTfloat wld_y1;
    LTfloat wld_x2;
    LTfloat wld_y2;
    if (nargs == 6) {
        LTfloat pix_w = ltGetPixelWidth();
        LTfloat pix_h = ltGetPixelHeight();
        LTfloat world_width = (LTfloat)w * pix_w;
        LTfloat world_height = (LTfloat)h * pix_h;
        wld_x1 = - world_width * 0.5f;
        wld_y1 = - world_height * 0.5f;
        wld_x2 = world_width * 0.5f;
        wld_y2 = world_height * 0.5f;
    } else {
        wld_x1 = luaL_checknumber(L, 7);
        wld_y1 = luaL_checknumber(L, 8);
        wld_x2 = luaL_checknumber(L, 9);
        wld_y2 = luaL_checknumber(L, 10);
    }

    LTRenderTarget *render_target = new LTRenderTarget(w, h,
        vp_x1, vp_y1, vp_x2, vp_y2,
        wld_x1, wld_y1, wld_x2, wld_y2,
        false, LT_TEXTURE_FILTER_LINEAR, LT_TEXTURE_FILTER_LINEAR);
    push_wrap(L, render_target);
    return 1;
}

/************************* Tweens **************************/

static int lt_MakeNativeTween(lua_State *L) {
    LTObject *obj = get_object(L, 1, LT_TYPE_OBJECT);
    const char *fname = lua_tostring(L, 2);
    if (fname == NULL) {
        lua_pushnil(L);
        return 1;
    }
    LTFieldDescriptor *field = obj->field(fname);
    if (field == NULL || field->offset < 0 || field->type != LT_FIELD_TYPE_FLOAT || field->access == LT_ACCESS_READONLY) {
        lua_pushnil(L);
        return 1;
    }
    LTfloat *fptr = (LTfloat*)((char*)obj + field->offset);
    LTfloat delay = luaL_checknumber(L, 3);
    LTfloat value = luaL_checknumber(L, 4);
    LTfloat time = luaL_checknumber(L, 5);
    LTEaseFunc ease_func = NULL;
    if (lua_isnil(L, 6)) {
        ease_func = ltEase_linear;
    } else {
        size_t len;
        const char *ease_func_str = lua_tolstring(L, 6, &len);
        if (ease_func_str == NULL) {
            lua_pushnil(L);
            return 1;
        }
        if (strcmp(ease_func_str, "in") == 0) {
            ease_func = ltEase_in;
        } else if (strcmp(ease_func_str, "out") == 0) {
            ease_func = ltEase_out;
        } else if (strcmp(ease_func_str, "inout") == 0) {
            ease_func = ltEase_inout;
        } else if (strcmp(ease_func_str, "zoomin") == 0) {
            ease_func = ltEase_zoomin;
        } else if (strcmp(ease_func_str, "zoomout") == 0) {
            ease_func = ltEase_zoomout;
        } else if (strcmp(ease_func_str, "accel") == 0) {
            ease_func = ltEase_accel;
        } else if (strcmp(ease_func_str, "decel") == 0) {
            ease_func = ltEase_decel;
        } else if (strcmp(ease_func_str, "bounce") == 0) {
            ease_func = ltEase_bounce;
        } else if (strcmp(ease_func_str, "revolve") == 0) {
            ease_func = ltEase_revolve;
        } else if (strcmp(ease_func_str, "backin") == 0) {
            ease_func = ltEase_backin;
        } else if (strcmp(ease_func_str, "backout") == 0) {
            ease_func = ltEase_backout;
        } else if (strcmp(ease_func_str, "linear") == 0) {
            ease_func = ltEase_linear;
        } else if (strcmp(ease_func_str, "elastic") == 0) {
            ease_func = ltEase_elastic;
        } else {
            return luaL_error(L, "Invalid easing function: ", ease_func_str);
        }
    }
    LTTween *tween = (LTTween*)lua_newuserdata(L, sizeof(LTTween));
    ltInitTween(tween, obj, fptr, value, time, delay, ease_func);
    return 1;
}

static int lt_AdvanceNativeTween(lua_State *L) {
    LTTween *tween = (LTTween*)lua_touserdata(L, 1);
    LTfloat dt = luaL_checknumber(L, 2);
    lua_pushboolean(L, ltAdvanceTween(tween, dt) ? 1 : 0);
    return 1;
}

/*
static int lt_TweenSet(lua_State *L) {
    push_wrap(L, new LTTweenSet());
    return 1;
}

static int lt_AdvanceTweens(lua_State *L) {
    check_nargs(L, 2);
    LTTweenSet *tweens = (LTTweenSet*)get_object(L, 1, LT_TYPE_TWEENSET);
    LTfloat dt = luaL_checknumber(L, 2);

    int i = 0;
    while (i < tweens->occupants) {
        LTTween *tween = &tweens->tweens[i];
        if (ltAdvanceTween(tween, dt)) {
            // Tween finished, so remove it.
            lua_pushlightuserdata(L, tween->field_ptr);
            lua_pushnil(L);
            lua_rawset(L, 1);
            push_wrap(L, tween->owner);
            del_ref(L, 1, -1);
            lua_pop(L, 1);
            if (i != tweens->occupants - 1) {
                tweens->tweens[i] = tweens->tweens[tweens->occupants - 1];
                // Update slot of tween that has taken the place of the deleted tween.
                lua_pushlightuserdata(L, tweens->tweens[i].field_ptr);
                lua_pushinteger(L, i);
                lua_rawset(L, 1);
            }
            tweens->occupants--;
        } else {
            i++;
        }
    }

    return 0;
}

static int lt_AddTween(lua_State *L) {
    check_nargs(L, 7);
    LTTweenSet *tweens = (LTTweenSet*)get_object(L, 1, LT_TYPE_TWEENSET);
    LTObject *obj = get_object(L, 2, LT_TYPE_OBJECT);
    const char *field = lua_tostring(L, 3);
    if (field == NULL) {
        return luaL_error(L, "Expecting a string in argument 3");
    }
    LTfloat *field_ptr = obj->field_ptr(field);
    if (field_ptr == NULL) {
        lua_pushboolean(L, 0);
        return 1;
    }
    LTfloat target_val = luaL_checknumber(L, 4);
    LTfloat time = luaL_checknumber(L, 5);
    LTfloat delay = luaL_checknumber(L, 6);
    size_t len;
    const char *ease_func_str = lua_tolstring(L, 7, &len);
    if (ease_func_str == NULL) {
        return luaL_error(L, "Expecting a string in argument 6");
    }
    LTEaseFuncInfo *ease_func_info = LTEaseFunc_lookup(ease_func_str, len);
    if (ease_func_info == NULL) {
        return luaL_error(L, "Invalid easing function: ", ease_func_str);
    }
    LTEaseFunc ease_func = ease_func_info->func;
    int slot = -1;
    lua_pushlightuserdata(L, field_ptr);
    lua_rawget(L, 1);
    if (!lua_isnil(L, -1)) {
        slot = lua_tonumber(L, -1);
    }
    lua_pop(L, 1);
    if (slot < 0) {
        slot = tweens->add(obj, field_ptr, target_val, time, delay, ease_func, slot);
        lua_pushlightuserdata(L, field_ptr);
        lua_pushinteger(L, slot);
        lua_rawset(L, 1); // Record the slot number of the field_ptr in the wrapper table.
    } else {
        tweens->add(obj, field_ptr, target_val, time, delay, ease_func, slot);
    }
    add_ref(L, 1, 2); // Add a reference to the field owner.
    lua_pushboolean(L, 1);
    return 1;
}

static int lt_ClearTweens(lua_State *L) {
    check_nargs(L, 1);
    LTTweenSet *tweens = (LTTweenSet*)get_object(L, 1, LT_TYPE_TWEENSET);
    for (int i = 0; i < tweens->occupants; i++) {
        LTTween *tween = &tweens->tweens[i];
        push_wrap(L, tween->owner);
        del_ref(L, 1, -1);
        lua_pop(L, 1);
        lua_pushlightuserdata(L, tween->field_ptr);
        lua_pushnil(L);
        lua_rawset(L, 1);
    }
    tweens->occupants = 0;
    return 0;
}
*/

/************************* Events **************************/

static bool call_pointer_event_handler(lua_State *L, int func, LTfloat x, LTfloat y, int input_id) {
    get_weak_ref(L, func);
    if (lua_isfunction(L, -1)) {
        lua_pushinteger(L, input_id);
        lua_pushnumber(L, x);
        lua_pushnumber(L, y);
        lua_call(L, 3, 1);
        bool consumed;
        if (lua_isnil(L, -1)) {
            consumed = true; // callback didn't return anything, so don't propogate by default.
        } else {
            consumed = lua_toboolean(L, -1);
        }
        lua_pop(L, 1);
        return consumed;
    } else {
        lua_pop(L, 1);
        return false;
    }
}

struct LTLPointerDownInEventHandler : LTPointerEventHandler {
    int lua_func_ref;

    LTLPointerDownInEventHandler(int func_index) {
        lua_func_ref = make_weak_ref(g_L, func_index);
    }

    virtual bool consume(LTfloat x, LTfloat y, LTSceneNode *node, LTPointerEvent *event) {
        if (event->type == LT_EVENT_POINTER_DOWN) {
            if (node->containsPoint(x, y)) {
                return call_pointer_event_handler(g_L, lua_func_ref, x, y, event->input_id);
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
};

struct LTLPointerUpEventHandler : LTPointerEventHandler {
    int lua_func_ref;

    LTLPointerUpEventHandler(int func_index) {
        lua_func_ref = make_weak_ref(g_L, func_index);
    }

    virtual bool consume(LTfloat x, LTfloat y, LTSceneNode *node, LTPointerEvent *event) {
        if (event->type == LT_EVENT_POINTER_UP) {
            return call_pointer_event_handler(g_L, lua_func_ref, x, y, event->input_id);
        } else {
            return false;
        }
    }
};

struct LTLPointerDownEventHandler : LTPointerEventHandler {
    int lua_func_ref;

    LTLPointerDownEventHandler(int func_index) {
        lua_func_ref = make_weak_ref(g_L, func_index);
    }

    virtual bool consume(LTfloat x, LTfloat y, LTSceneNode *node, LTPointerEvent *event) {
        if (event->type == LT_EVENT_POINTER_DOWN) {
            return call_pointer_event_handler(g_L, lua_func_ref, x, y, event->input_id);
        } else {
            return false;
        }
    }
};

struct LTLPointerMoveEventHandler : LTPointerEventHandler {
    int lua_func_ref;

    LTLPointerMoveEventHandler(int func_index) {
        lua_func_ref = make_weak_ref(g_L, func_index);
    }

    virtual bool consume(LTfloat x, LTfloat y, LTSceneNode *node, LTPointerEvent *event) {
        if (event->type == LT_EVENT_POINTER_MOVE) {
            return call_pointer_event_handler(g_L, lua_func_ref, x, y, event->input_id);
        } else {
            return false;
        }
    }
};

struct LTLPointerOverEventHandler : LTPointerEventHandler {
    int lua_enter_func_ref;
    int lua_exit_func_ref;
    bool first_time;
    bool in;

    LTLPointerOverEventHandler(int enter_func_index, int exit_func_index) {
        lua_enter_func_ref = make_weak_ref(g_L, enter_func_index);
        lua_exit_func_ref = make_weak_ref(g_L, exit_func_index);
        first_time = true;
        in = false;
    }

    virtual bool consume(LTfloat x, LTfloat y, LTSceneNode *node, LTPointerEvent *event) {
        if (event->type == LT_EVENT_POINTER_MOVE) {
            bool containsPoint = node->containsPoint(x, y);
            if (first_time) {
                first_time = false;
                in = containsPoint;
                if (in) {
                    return call_pointer_event_handler(g_L, lua_enter_func_ref, x, y, event->input_id);
                } else {
                    return false;
                }
            } else {
                bool res = false;
                if (containsPoint && !in) {
                    res = call_pointer_event_handler(g_L, lua_enter_func_ref, x, y, event->input_id);
                } else if (!containsPoint && in) {
                    res = call_pointer_event_handler(g_L, lua_exit_func_ref, x, y, event->input_id);
                }
                in = containsPoint;
                return res;
            }
        } else {
            return false;
        }
    }
};

static int lt_AddOnPointerUpHandler(lua_State *L) {
    check_nargs(L, 2);
    LTSceneNode *node = (LTSceneNode*)get_object(L, 1, LT_TYPE_SCENENODE);
    LTLPointerUpEventHandler *handler = new LTLPointerUpEventHandler(2);
    node->addHandler(handler);
    add_ref(L, 1, 2);
    return 0;
}

static int lt_AddOnPointerDownHandler(lua_State *L) {
    check_nargs(L, 2);
    LTSceneNode *node = (LTSceneNode*)get_object(L, 1, LT_TYPE_SCENENODE);
    LTLPointerDownEventHandler *handler = new LTLPointerDownEventHandler(2);
    node->addHandler(handler);
    add_ref(L, 1, 2);
    return 0;
}

static int lt_AddOnPointerMoveHandler(lua_State *L) {
    check_nargs(L, 2);
    LTSceneNode *node = (LTSceneNode*)get_object(L, 1, LT_TYPE_SCENENODE);
    LTLPointerMoveEventHandler *handler = new LTLPointerMoveEventHandler(2);
    node->addHandler(handler);
    add_ref(L, 1, 2);
    return 0;
}

static int lt_AddOnPointerOverHandler(lua_State *L) {
    check_nargs(L, 3);
    LTSceneNode *node = (LTSceneNode*)get_object(L, 1, LT_TYPE_SCENENODE);
    LTLPointerOverEventHandler *handler = new LTLPointerOverEventHandler(2, 3);
    node->addHandler(handler);
    add_ref(L, 1, 2);
    add_ref(L, 1, 3);
    return 0;
}

static int lt_PropogatePointerUpEvent(lua_State *L) {
    check_nargs(L, 4);
    LTSceneNode *node = (LTSceneNode*)get_object(L, 1, LT_TYPE_SCENENODE);
    int input_id = luaL_checkinteger(L, 2);
    LTfloat x = luaL_checknumber(L, 3);
    LTfloat y = luaL_checknumber(L, 4);
    LTPointerEvent event(LT_EVENT_POINTER_UP, x, y, input_id);
    node->propogatePointerEvent(x, y, &event);
    return 0;
}

static int lt_PropogatePointerDownEvent(lua_State *L) {
    check_nargs(L, 4);
    LTSceneNode *node = (LTSceneNode*)get_object(L, 1, LT_TYPE_SCENENODE);
    int input_id = luaL_checkinteger(L, 2);
    LTfloat x = luaL_checknumber(L, 3);
    LTfloat y = luaL_checknumber(L, 4);
    LTPointerEvent event(LT_EVENT_POINTER_DOWN, x, y, input_id);
    node->propogatePointerEvent(x, y, &event);
    return 0;
}

static int lt_PropogatePointerMoveEvent(lua_State *L) {
    check_nargs(L, 4);
    LTSceneNode *node = (LTSceneNode*)get_object(L, 1, LT_TYPE_SCENENODE);
    int input_id = luaL_checkinteger(L, 2);
    LTfloat x = luaL_checknumber(L, 3);
    LTfloat y = luaL_checknumber(L, 4);
    LTPointerEvent event(LT_EVENT_POINTER_MOVE, x, y, input_id);
    node->propogatePointerEvent(x, y, &event);
    return 0;
}

/************************* Images **************************/

static void add_packer_images_to_lua_table(lua_State *L, int w, int h, LTImagePacker *packer, LTAtlas *atlas) {
    const char *name;
    if (packer->occupant != NULL) {
        LTImage *img = new LTImage(atlas, w, h, packer);
        name = packer->occupant->name;
        if (!packer->occupant->is_glyph) {
            push_wrap(L, img);
            lua_setfield(L, -2, name);
        } else {
            lua_getfield(L, -1, name);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                lua_newtable(L);
                lua_pushvalue(L, -1);
                lua_setfield(L, -3, name);
            }
            // Font table now on top of stack.
            char glyph_name[2];
            glyph_name[0] = packer->occupant->glyph_char;
            glyph_name[1] = '\0';
            push_wrap(L, img);
            lua_setfield(L, -2, glyph_name);
            lua_pop(L, 1); // Pop font table.
        }
        add_packer_images_to_lua_table(L, w, h, packer->lo_child, atlas);
        add_packer_images_to_lua_table(L, w, h, packer->hi_child, atlas);
    }
}

#define MIN_TEX_SIZE 64

static void pack_image(lua_State *L, LTImagePacker *packer, LTImageBuffer *buf,
        LTTextureFilter minfilter, LTTextureFilter magfilter) {
    if (!ltPackImage(packer, buf)) {
        // Packer full, so generate an atlas.
        LTAtlas *atlas = new LTAtlas(packer, minfilter, magfilter);
        add_packer_images_to_lua_table(L, packer->width, packer->height, packer, atlas);
        packer->deleteOccupants();
        packer->width = MIN_TEX_SIZE;
        packer->height = MIN_TEX_SIZE;

        if (!ltPackImage(packer, buf)) {
            luaL_error(L, "Image %s is too large (%dx%d).", buf->name, buf->bb_width(), buf->bb_height());
        }
    }
}

LTTextureFilter decode_texture_filter_arg(lua_State *L, int arg) {
    const char *str = lua_tostring(L, arg);
    if (str == NULL) {
        luaL_error(L, "Expecting a string in argument %d", arg);
    }
    if (strcmp(str, "nearest") == 0) {
        return LT_TEXTURE_FILTER_NEAREST;
    }
    if (strcmp(str, "linear") == 0) {
        return LT_TEXTURE_FILTER_LINEAR;
    }
    luaL_error(L, "Unrecognised texture filter: %s", str);
    return LT_TEXTURE_FILTER_LINEAR; // unreachable
}

static int lt_LoadImages(lua_State *L) {
    // Load images in 1st argument (an array) and return a table
    // indexed by image name.
    // The second and third arguments are the minimize and magnify
    // texture filters to use.
    // If an entry in the array is a table, then process it as a font.
    int num_args = check_nargs(L, 1);
    LTTextureFilter minfilter = LT_TEXTURE_FILTER_LINEAR;
    LTTextureFilter magfilter = LT_TEXTURE_FILTER_LINEAR;
    if (num_args > 1) {
        minfilter = decode_texture_filter_arg(L, 2);
    }
    if (num_args > 2) {
        magfilter = decode_texture_filter_arg(L, 3);
    }
    lua_newtable(L); // The table to be returned.
    LTImagePacker *packer = new LTImagePacker(0, 0, MIN_TEX_SIZE, MIN_TEX_SIZE, max_atlas_size());
    int i = 1;
    while (true) {
        lua_pushinteger(L, i);
        lua_gettable(L, 1);
        if (lua_isnil(L, -1)) {
            // We've reached the end of the array.
            lua_pop(L, 1);
            break;
        }
        if (lua_isstring(L, -1)) {
            const char* name = lua_tostring(L, -1);
            lua_pop(L, 1);
            if (name == NULL) {
                return luaL_error(L, "Expecting an array of strings.");
            }
            const char *path = image_path(name); 
            LTImageBuffer *buf = ltReadImage(path, name);
            delete[] path;
            if (buf != NULL) {
                // If buf is NULL ltReadImage would have already logged an error.
                pack_image(L, packer, buf, minfilter, magfilter);
            }
        } else if (lua_istable(L, -1)) {
            // A table entry means we should load the image as a font.
            lua_getfield(L, -1, "font");
            const char *name = lua_tostring(L, -1);
            lua_pop(L, 1);
            if (name == NULL) {
                return luaL_error(L, "Expecting a font field in table entry.");
            }
            lua_getfield(L, -1, "glyphs");
            const char *glyphs = lua_tostring(L, -1);
            lua_pop(L, 1);
            if (glyphs == NULL) {
                return luaL_error(L, "Expecting a glyphs field in table entry.");
            }
            lua_pop(L, 1); // Pop table entry.
            const char *path = image_path(name); 
            LTImageBuffer *buf = ltReadImage(path, name);
            delete[] path;
            if (buf != NULL) {
                // If buf is NULL ltReadImage would already have logged an error.
                std::list<LTImageBuffer *> *glyph_list = ltImageBufferToGlyphs(buf, glyphs);
                delete buf;
                std::list<LTImageBuffer *>::iterator it;
                for (it = glyph_list->begin(); it != glyph_list->end(); it++) {
                    pack_image(L, packer, *it, minfilter, magfilter);
                }
                delete glyph_list;
            }
        } else {
            return luaL_error(L, "Entries must be strings or tables");
        }

        i++;
    }

    // Pack any images left in packer into a new texture.
    if (packer->size() > 0) {
        LTAtlas *atlas = new LTAtlas(packer, minfilter, magfilter);
        add_packer_images_to_lua_table(L, packer->width, packer->height, packer, atlas);
        packer->deleteOccupants();
    }
        
    delete packer;

    return 1;
}

/************************* Audio **************************/

static int lt_LoadSamples(lua_State *L) {
    // Load sounds in 1st argument (an array) and return a table
    // indexed by sound name.
    check_nargs(L, 1);
    lua_newtable(L); // The table to be returned.
    int i = 1;
    while (true) {
        lua_pushinteger(L, i);
        lua_gettable(L, 1);
        // The top of the stack now contains the ith entry of the array argument.
        if (lua_isnil(L, -1)) {
            // We've reached the end of the array.
            lua_pop(L, 1);
            break;
        }
        const char* name = lua_tostring(L, -1);
        lua_pop(L, 1);
        // The top of the stack now contains the table to be returned.
        if (name == NULL) {
            return luaL_error(L, "Expecting an array of strings.");
        }
        const char *path = sound_path(name); 
        LTAudioSample *sample = ltReadAudioSample(path, name);
        delete[] path;
        if (sample != NULL) {
            // If sample is NULL ltReadAudioSample would have already logged an error.
            push_wrap(L, sample);
            lua_setfield(L, -2, name);
        }
        i++;
    }
    return 1;
}

static int lt_PlaySampleOnce(lua_State *L) {
    int num_args = check_nargs(L, 1);
    LTfloat pitch = 1.0f;
    LTfloat gain = 1.0f;
    if (num_args > 1) {
        pitch = luaL_checknumber(L, 2);
    }
    if (num_args > 2) {
        gain = luaL_checknumber(L, 3);
    }
    LTAudioSample *sample = (LTAudioSample*)get_object(L, 1, LT_TYPE_AUDIOSAMPLE);
    sample->play(pitch, gain);
    return 0;
}

static int lt_PlayTrack(lua_State *L) {
    check_nargs(L, 1);
    LTTrack *track = (LTTrack*)get_object(L, 1, LT_TYPE_TRACK);
    track->play();
    return 0;
}

static int lt_PauseTrack(lua_State *L) {
    check_nargs(L, 1);
    LTTrack *track = (LTTrack*)get_object(L, 1, LT_TYPE_TRACK);
    track->pause();
    return 0;
}

static int lt_StopTrack(lua_State *L) {
    check_nargs(L, 1);
    LTTrack *track = (LTTrack*)get_object(L, 1, LT_TYPE_TRACK);
    track->stop();
    return 0;
}

static int lt_RewindTrack(lua_State *L) {
    check_nargs(L, 1);
    LTTrack *track = (LTTrack*)get_object(L, 1, LT_TYPE_TRACK);
    track->rewind();
    return 0;
}

static int lt_QueueSampleInTrack(lua_State *L) {
    int num_args = check_nargs(L, 2);
    LTTrack *track = (LTTrack*)get_object(L, 1, LT_TYPE_TRACK);
    LTAudioSample *sample = (LTAudioSample*)get_object(L, 2, LT_TYPE_AUDIOSAMPLE);
    int n = 1;
    if (num_args > 2) {
        n = luaL_checkinteger(L, 3);
    }
    for (int i = 0; i < n; i++) {
        track->queueSample(sample);
    }
    // XXX We should probably add a separate reference to each added
    //     sample, so we can remove each reference in lt_TrackDequeuePlayed.
    add_ref(L, 1, 2); // Add ref from track to sample.
    return 0;
}

static int lt_Track(lua_State *L) {
    LTTrack *track = new LTTrack();
    push_wrap(L, track);
    return 1;
}

static int lt_SetTrackLoop(lua_State *L) {
    check_nargs(L, 2);
    LTTrack *track = (LTTrack*)get_object(L, 1, LT_TYPE_TRACK);
    bool loop = lua_toboolean(L, 2);
    track->setLoop(loop);
    return 0;
}

static int lt_TrackQueueSize(lua_State *L) {
    check_nargs(L, 1);
    LTTrack *track = (LTTrack*)get_object(L, 1, LT_TYPE_TRACK);
    lua_pushinteger(L, track->numSamples());
    return 1;
}

static int lt_TrackNumPlayed(lua_State *L) {
    check_nargs(L, 1);
    LTTrack *track = (LTTrack*)get_object(L, 1, LT_TYPE_TRACK);
    lua_pushinteger(L, track->numProcessedSamples());
    return 1;
}

static int lt_TrackNumPending(lua_State *L) {
    check_nargs(L, 1);
    LTTrack *track = (LTTrack*)get_object(L, 1, LT_TYPE_TRACK);
    lua_pushinteger(L, track->numPendingSamples());
    return 1;
}

static int lt_TrackDequeuePlayed(lua_State *L) {
    check_nargs(L, 2);
    LTTrack *track = (LTTrack*)get_object(L, 1, LT_TYPE_TRACK);
    int n = (int)luaL_checkinteger(L, 2);
    track->dequeueProcessedSamples(n);
    // XXX We have to remove the reference to the sample.
    return 0;
}

static int lt_SampleNumDataPoints(lua_State *L) {
    check_nargs(L, 1);
    LTAudioSample *sample = (LTAudioSample*)get_object(L, 1, LT_TYPE_AUDIOSAMPLE);
    lua_pushinteger(L, sample->numDataPoints());
    return 1;
}

static int lt_SampleFrequency(lua_State *L) {
    check_nargs(L, 1);
    LTAudioSample *sample = (LTAudioSample*)get_object(L, 1, LT_TYPE_AUDIOSAMPLE);
    lua_pushinteger(L, sample->dataPointsPerSec());
    return 1;
}

static int lt_SampleLength(lua_State *L) {
    check_nargs(L, 1);
    LTAudioSample *sample = (LTAudioSample*)get_object(L, 1, LT_TYPE_AUDIOSAMPLE);
    lua_pushnumber(L, sample->length());
    return 1;
}

/************************* Box2D **************************/

static int lt_FixtureContainsPoint(lua_State *L) {
    check_nargs(L, 3); 
    LTFixture *fixture = (LTFixture*)get_object(L, 1, LT_TYPE_FIXTURE);
    LTfloat x = luaL_checknumber(L, 2);
    LTfloat y = luaL_checknumber(L, 3);
    if (fixture->fixture != NULL) {
        lua_pushboolean(L, fixture->fixture->TestPoint(b2Vec2(x, y)));
    } else {
        lua_pushboolean(L, 0);
    }
    return 1;
}

static int lt_DestroyFixture(lua_State *L) {
    check_nargs(L, 1); 
    LTFixture *fixture = (LTFixture*)get_object(L, 1, LT_TYPE_FIXTURE);
    fixture->destroy();
    LTBody *body = fixture->body;
    if (body != NULL) {
        get_weak_ref(L, fixture->lua_wrap);
        get_weak_ref(L, body->lua_wrap);
        del_ref(L, -1, -2); // Remove reference from body to fixture.
        del_ref(L, -2, -1); // Remove reference from fixture to body.
        lua_pop(L, 2);
    }
    return 0;
}

static int lt_FixtureIsDestroyed(lua_State *L) {
    check_nargs(L, 1); 
    LTFixture *fixture = (LTFixture*)get_object(L, 1, LT_TYPE_FIXTURE);
    lua_pushboolean(L, fixture->fixture == NULL);
    return 1;
}

static int lt_DoWorldStep(lua_State *L) {
    int num_args = check_nargs(L, 2); 
    LTWorld *world = (LTWorld*)get_object(L, 1, LT_TYPE_WORLD);
    LTfloat time_step = luaL_checknumber(L, 2);
    int velocity_iterations = 8;
    int position_iterations = 3;
    if (num_args > 2) {
        velocity_iterations = luaL_checkinteger(L, 3);
    }
    if (num_args > 3) {
        position_iterations = luaL_checkinteger(L, 4);
    }
    world->world->Step(time_step, velocity_iterations, position_iterations);
    return 0;
}

static int lt_SetWorldGravity(lua_State *L) {
    check_nargs(L, 3);
    LTWorld *world = (LTWorld*)get_object(L, 1, LT_TYPE_WORLD);
    LTfloat x = (LTfloat)luaL_checknumber(L, 2);
    LTfloat y = (LTfloat)luaL_checknumber(L, 3);
    world->world->SetGravity(b2Vec2(x, y));
    return 0;
}

static int lt_SetWorldAutoClearForces(lua_State *L) {
    check_nargs(L, 2);
    LTWorld *world = (LTWorld*)get_object(L, 1, LT_TYPE_WORLD);
    bool clear = lua_toboolean(L, 2);
    world->world->SetAutoClearForces(clear);
    return 0;
}

struct AABBQueryCallBack : b2QueryCallback {
    lua_State *L;
    int i;

    AABBQueryCallBack(lua_State *L) {
        AABBQueryCallBack::L = L;
        i = 1;
    }

    virtual bool ReportFixture(b2Fixture *fixture) {
        LTFixture *f = (LTFixture*)fixture->GetUserData();
        push_wrap(L, f);
        lua_rawseti(L, -2, i);
        i++;
        return true;
    }
};

static int lt_WorldQueryBox(lua_State *L) {
    check_nargs(L, 5);
    LTWorld *world = (LTWorld*)get_object(L, 1, LT_TYPE_WORLD);
    LTfloat x1 = (LTfloat)luaL_checknumber(L, 2);
    LTfloat y1 = (LTfloat)luaL_checknumber(L, 3);
    LTfloat x2 = (LTfloat)luaL_checknumber(L, 4);
    LTfloat y2 = (LTfloat)luaL_checknumber(L, 5);
    b2AABB aabb;
    if (x1 > x2) {
        aabb.upperBound.x = x1;
        aabb.lowerBound.x = x2;
    } else {
        aabb.upperBound.x = x2;
        aabb.lowerBound.x = x1;
    }
    if (y1 > y2) {
        aabb.upperBound.y = y1;
        aabb.lowerBound.y = y2;
    } else {
        aabb.upperBound.y = y2;
        aabb.lowerBound.y = y1;
    }
    AABBQueryCallBack cb(L);
    lua_newtable(L);
    world->world->QueryAABB(&cb, aabb);
    return 1;
}

static int lt_DestroyBody(lua_State *L) {
    check_nargs(L, 1);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    body->destroy();
    b2Body *b = body->body;
    if (b != NULL) {
        // Remove references between body and its fixtures.
        b2Fixture *f = b->GetFixtureList();
        while (f != NULL) {
            LTFixture *ud = (LTFixture*)f->GetUserData();
            get_weak_ref(L, ud->lua_wrap);
            del_ref(L, 1, -1); // Remove reference from body to fixture.
            del_ref(L, -1, 1); // Remove reference from fixture to body.
            lua_pop(L, 1);
            f = f->GetNext();
        }
    }
    LTWorld *world = body->world;
    get_weak_ref(L, world->lua_wrap);
    del_ref(L, -1, 1); // Remove reference from world to body.
    del_ref(L, 1, -1); // Remove reference from body to world.
    lua_pop(L, 1);
    return 0;
}

static int lt_BodyIsDestroyed(lua_State *L) {
    check_nargs(L, 1);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    lua_pushboolean(L, body->body == NULL);
    return 1;
}

static int lt_ApplyForceToBody(lua_State *L) {
    int num_args = check_nargs(L, 3);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        b2Vec2 force;
        b2Vec2 pos;
        force.x = luaL_checknumber(L, 2);
        force.y = (LTfloat)luaL_checknumber(L, 3);
        if (num_args >= 5) {
            pos.x = (LTfloat)luaL_checknumber(L, 4);
            pos.y = (LTfloat)luaL_checknumber(L, 5);
        } else {
            pos = body->body->GetWorldCenter();
        }
        body->body->ApplyForce(force, pos);
    }
    return 0;
}

static int lt_ApplyTorqueToBody(lua_State *L) {
    check_nargs(L, 2);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        body->body->ApplyTorque(luaL_checknumber(L, 2));
    }
    return 0;
}

static int lt_ApplyImpulseToBody(lua_State *L) {
    int num_args = check_nargs(L, 3);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        b2Vec2 force;
        b2Vec2 pos;
        force.x = luaL_checknumber(L, 2);
        force.y = (LTfloat)luaL_checknumber(L, 3);
        if (num_args >= 5) {
            pos.x = (LTfloat)luaL_checknumber(L, 4);
            pos.y = (LTfloat)luaL_checknumber(L, 5);
        } else {
            pos = body->body->GetWorldCenter();
        }
        body->body->ApplyLinearImpulse(force, pos);
    }
    return 0;
}

static int lt_ApplyAngularImpulseToBody(lua_State *L) {
    check_nargs(L, 2);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        body->body->ApplyAngularImpulse(luaL_checknumber(L, 2));
    }
    return 0;
}

/*
 * m_forces and m_torque are private members.
static int lt_ClearBodyForces(lua_State *L) {
    check_nargs(L, 1);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        body->body->m_force.SetZero();
        body->body->m_torque = 0.0f;
    }
    return 0;
}
*/

static int lt_GetBodyAngle(lua_State *L) {
    check_nargs(L, 1);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        lua_pushnumber(L, body->body->GetAngle() * LT_DEGREES_PER_RADIAN);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lt_SetBodyAngle(lua_State *L) {
    check_nargs(L, 2);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        body->body->SetTransform(body->body->GetPosition(), luaL_checknumber(L, 2) * LT_RADIANS_PER_DEGREE);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lt_GetBodyPosition(lua_State *L) {
    check_nargs(L, 1);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        b2Vec2 pos = body->body->GetPosition();
        lua_pushnumber(L, pos.x);
        lua_pushnumber(L, pos.y);
        return 2;
    }
    return 0;
}

static int lt_GetBodyVelocity(lua_State *L) {
    check_nargs(L, 1);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        b2Vec2 pos = body->body->GetLinearVelocity();
        lua_pushnumber(L, pos.x);
        lua_pushnumber(L, pos.y);
        return 2;
    }
    return 0;
}

static int lt_SetBodyVelocity(lua_State *L) {
    check_nargs(L, 3);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        b2Vec2 v = b2Vec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
        body->body->SetLinearVelocity(v);
    }
    return 0;
}

static int lt_SetBodyPosition(lua_State *L) {
    check_nargs(L, 3);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    LTfloat x = luaL_checknumber(L, 2);
    LTfloat y = luaL_checknumber(L, 3);
    if (body->body != NULL) {
        body->body->SetTransform(b2Vec2(x, y), body->body->GetAngle());
    }
    return 0;
}

static int lt_SetBodyAngularVelocity(lua_State *L) {
    check_nargs(L, 2);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        body->body->SetAngularVelocity(luaL_checknumber(L, 2) * LT_RADIANS_PER_DEGREE);
    }
    return 0;
}

static int lt_SetBodyGravityScale(lua_State *L) {
    check_nargs(L, 2);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        body->body->SetGravityScale(luaL_checknumber(L, 2));
    }
    return 0;
}

static int lt_AddRectToBody(lua_State *L) {
    int num_args = check_nargs(L, 5);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        LTfloat x1 = (LTfloat)luaL_checknumber(L, 2);
        LTfloat y1 = (LTfloat)luaL_checknumber(L, 3);
        LTfloat x2 = (LTfloat)luaL_checknumber(L, 4);
        LTfloat y2 = (LTfloat)luaL_checknumber(L, 5);
        LTfloat density = 0.0f;
        if (num_args > 5) {
            density = (LTfloat)luaL_checknumber(L, 6);
        }
        b2PolygonShape poly;
        poly.m_vertexCount = 4;
        poly.m_vertices[0].Set(x1, y1);
        poly.m_vertices[1].Set(x2, y1);
        poly.m_vertices[2].Set(x2, y2);
        poly.m_vertices[3].Set(x1, y2);
        poly.m_normals[0].Set(0.0f, -1.0f);
        poly.m_normals[1].Set(1.0f, 0.0f);
        poly.m_normals[2].Set(0.0f, 1.0f);
        poly.m_normals[3].Set(-1.0f, 0.0f);
        poly.m_centroid.Set(x1 + ((x2 - x1) * 0.5f), y1 + ((y2 - y1) * 0.5f));
        b2FixtureDef fixtureDef;
        fixtureDef.density = density;
        fixtureDef.shape = &poly;
        LTFixture *fixture = new LTFixture(body, &fixtureDef);
        push_wrap(L, fixture);
        add_ref(L, 1, -1); // Add reference from body to new fixture.
        add_ref(L, -1, 1); // Add reference from fixture to body.
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lt_AddTriangleToBody(lua_State *L) {
    int num_args = check_nargs(L, 7);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        LTfloat x1 = (LTfloat)luaL_checknumber(L, 2);
        LTfloat y1 = (LTfloat)luaL_checknumber(L, 3);
        LTfloat x2 = (LTfloat)luaL_checknumber(L, 4);
        LTfloat y2 = (LTfloat)luaL_checknumber(L, 5);
        LTfloat x3 = (LTfloat)luaL_checknumber(L, 6);
        LTfloat y3 = (LTfloat)luaL_checknumber(L, 7);
        LTfloat density = 0.0f;
        if (num_args > 7) {
            density = (LTfloat)luaL_checknumber(L, 8);
        }
        b2PolygonShape poly;
        b2Vec2 vertices[3];
        vertices[0].Set(x1, y1);
        vertices[1].Set(x2, y2);
        vertices[2].Set(x3, y3);
        if (!ltCheckB2Poly(vertices, 3)) {
            vertices[2] = vertices[0];
            vertices[0].Set(x3, y3);
            if (!ltCheckB2Poly(vertices, 3)) {
                lua_pushnil(L);
                return 1;
            }
        }
        poly.Set(vertices, 3);
        b2FixtureDef fixtureDef;
        fixtureDef.density = density;
        fixtureDef.shape = &poly;
        LTFixture *fixture = new LTFixture(body, &fixtureDef);
        push_wrap(L, fixture);
        add_ref(L, 1, -1); // Add reference from body to new fixture.
        add_ref(L, -1, 1); // Add reference from fixture to body.
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static void read_fixture_attributes(lua_State *L, int table, b2FixtureDef *fixture_def) {
    if (!lua_istable(L, table)) {
        luaL_error(L, "Expecting a table in position %d", table);
    }
    lua_getfield(L, table, "friction");
    if (!lua_isnil(L, -1)) {
        fixture_def->friction = luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);
    lua_getfield(L, table, "restitution");
    if (!lua_isnil(L, -1)) {
        fixture_def->restitution = luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);
    lua_getfield(L, table, "density");
    if (!lua_isnil(L, -1)) {
        fixture_def->density = luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);
    lua_getfield(L, table, "category");
    if (!lua_isnil(L, -1)) {
        fixture_def->filter.categoryBits = lua_tointeger(L, -1);
    }
    lua_pop(L, 1);
    lua_getfield(L, table, "mask");
    if (!lua_isnil(L, -1)) {
        fixture_def->filter.maskBits = lua_tointeger(L, -1);
    }
    lua_pop(L, 1);
    lua_getfield(L, table, "group");
    if (!lua_isnil(L, -1)) {
        fixture_def->filter.groupIndex = lua_tointeger(L, -1);
    }
    lua_pop(L, 1);
    lua_getfield(L, table, "sensor");
    if (!lua_isnil(L, -1)) {
        fixture_def->isSensor = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);
}

static int lt_AddPolygonToBody(lua_State *L) {
    check_nargs(L, 3);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        // Second argument is array of polygon vertices.
        if (!lua_istable(L, 2)) {
            return luaL_error(L, "Expecting an array in second argument");
        }
        int i = 1;
        int num_vertices = 0;
        b2PolygonShape poly;
        b2Vec2 vertices[b2_maxPolygonVertices];
        while (num_vertices < b2_maxPolygonVertices) {
            lua_rawgeti(L, 2, i);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                break;
            }
            vertices[num_vertices].x = luaL_checknumber(L, -1);
            lua_pop(L, 1);
            i++;
            lua_rawgeti(L, 2, i);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                break;
            }
            vertices[num_vertices].y = luaL_checknumber(L, -1);
            lua_pop(L, 1);
            i++;
            num_vertices++;
        }
        if (!ltCheckB2Poly(vertices, num_vertices)) {
            // Reverse vertices.
            for (int j = 0; j < (num_vertices >> 1); j++) {
                b2Vec2 tmp = vertices[j];
                vertices[j] = vertices[num_vertices - j - 1];
                vertices[num_vertices - j - 1] = tmp;
            }
            if (!ltCheckB2Poly(vertices, num_vertices)) {
                lua_pushnil(L);
                return 1;
            }
        }
        poly.Set(vertices, num_vertices);

        // Third argument is a table of fixture properties.
        b2FixtureDef fixture_def;
        read_fixture_attributes(L, 3, &fixture_def);
        fixture_def.shape = &poly;
        LTFixture *fixture = new LTFixture(body, &fixture_def);
        push_wrap(L, fixture);
        add_ref(L, 1, -1); // Add reference from body to new fixture.
        set_ref_field(L, -1, "body", 1); // Add reference from fixture to body.
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lt_AddCircleToBody(lua_State *L) {
    check_nargs(L, 5);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        LTfloat radius = luaL_checknumber(L, 2);
        LTfloat x = luaL_checknumber(L, 3);
        LTfloat y = luaL_checknumber(L, 4);
        b2CircleShape circle;
        circle.m_radius = radius;
        circle.m_p.Set(x, y);

        b2FixtureDef fixture_def;
        read_fixture_attributes(L, 5, &fixture_def);
        fixture_def.shape = &circle;
        LTFixture *fixture = new LTFixture(body, &fixture_def);
        push_wrap(L, fixture);
        add_ref(L, 1, -1); // Add reference from body to new fixture.
        set_ref_field(L, -1, "body", 1); // Add reference from fixture to body.
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lt_GetFixtureBody(lua_State *L) {
    check_nargs(L, 1);
    LTFixture *fixture = (LTFixture*)get_object(L, 1, LT_TYPE_FIXTURE);
    if (fixture->fixture != NULL && fixture->body != NULL) {
        push_wrap(L, fixture->body);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lt_GetBodyFixtures(lua_State *L) {
    check_nargs(L, 1);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    lua_newtable(L);
    b2Body *b = body->body;
    if (b) {
        b2Fixture* f = b->GetFixtureList();
        int i = 1;
        while (f != NULL) {
            push_wrap(L, (LTFixture*)f->GetUserData());
            lua_rawseti(L, -2, i);
            f = f->GetNext();
            i++;
        }
    }
    return 1;
}

static int lt_FixtureBoundingBox(lua_State *L) {
    check_nargs(L, 1);
    LTFixture *fixture = (LTFixture*)get_object(L, 1, LT_TYPE_FIXTURE);
    b2Fixture *f = fixture->fixture;
    if (f) {
        b2AABB aabb = f->GetAABB(0);
        lua_pushnumber(L, aabb.lowerBound.x);
        lua_pushnumber(L, aabb.lowerBound.y);
        lua_pushnumber(L, aabb.upperBound.x);
        lua_pushnumber(L, aabb.upperBound.y);
        return 4;
    } else {
        return 0;
    }
}

static int lt_AddStaticBodyToWorld(lua_State *L) {
    check_nargs(L, 1);
    LTWorld *world = (LTWorld*)get_object(L, 1, LT_TYPE_WORLD);
    b2BodyDef def;
    def.type = b2_staticBody;
    LTBody *body = new LTBody(world, &def);
    push_wrap(L, body);
    add_ref(L, 1, -1); // Add reference from world to body.
    add_ref(L, -1, 1); // Add reference from body to world.
    return 1;
}

static int lt_AddDynamicBodyToWorld(lua_State *L) {
    int num_args = check_nargs(L, 3);
    LTWorld *world = (LTWorld*)get_object(L, 1, LT_TYPE_WORLD);
    LTfloat x = (LTfloat)luaL_checknumber(L, 2);
    LTfloat y = (LTfloat)luaL_checknumber(L, 3);
    LTfloat angle = 0.0f;
    if (num_args > 3) {
        angle = (LTfloat)luaL_checknumber(L, 4);
    }
    b2BodyDef def;
    def.type = b2_dynamicBody;
    def.position.Set(x, y);
    def.angle = angle;
    LTBody *body = new LTBody(world, &def);
    push_wrap(L, body);
    add_ref(L, 1, -1); // Add reference from world to body.
    add_ref(L, -1, 1); // Add reference from body to world.
    return 1;
}

static int lt_AddBodyToWorld(lua_State *L) {
    check_nargs(L, 2);
    LTWorld *world = (LTWorld*)get_object(L, 1, LT_TYPE_WORLD);
    // Second argument is a table of body properties.
    b2BodyDef body_def;

    lua_getfield(L, 2, "type");
    if (!lua_isnil(L, -1)) {
        const char *type = luaL_checkstring(L, -1);
        b2BodyType btype;
        if (strcmp(type, "dynamic") == 0) {
            btype = b2_dynamicBody;
        } else if (strcmp(type, "static") == 0) {
            btype = b2_staticBody;
        } else if (strcmp(type, "kinematic") == 0) {
            btype = b2_kinematicBody;
        } else {
            return luaL_error(L, "Unknown body type: %s", type);
        }
        body_def.type = btype;
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "position");
    if (!lua_isnil(L, -1)) {
        if (lua_istable(L, -1)) {
            lua_rawgeti(L, -1, 1);
            body_def.position.x = luaL_checknumber(L, -1);
            lua_pop(L, 1);
            lua_rawgeti(L, -1, 2);
            body_def.position.y = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        } else {
            return luaL_error(L, "Expecting position field to be a table");
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "angle");
    if (!lua_isnil(L, -1)) {
        body_def.angle = LT_RADIANS_PER_DEGREE * luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "velocity");
    if (!lua_isnil(L, -1)) {
        if (lua_istable(L, -1)) {
            lua_rawgeti(L, -1, 1);
            body_def.linearVelocity.x = luaL_checknumber(L, -1);
            lua_pop(L, 1);
            lua_rawgeti(L, -1, 2);
            body_def.linearVelocity.y = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        } else {
            return luaL_error(L, "Expecting position field to be a table");
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "angular_velocity");
    if (!lua_isnil(L, -1)) {
        body_def.angularVelocity = LT_RADIANS_PER_DEGREE * luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "damping");
    if (!lua_isnil(L, -1)) {
        body_def.linearDamping = luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "angular_damping");
    if (!lua_isnil(L, -1)) {
        body_def.angularDamping = LT_RADIANS_PER_DEGREE * luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "can_sleep");
    if (!lua_isnil(L, -1)) {
        body_def.allowSleep = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "awake");
    if (!lua_isnil(L, -1)) {
        body_def.awake = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "fixed_rotation");
    if (!lua_isnil(L, -1)) {
        body_def.fixedRotation = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "bullet");
    if (!lua_isnil(L, -1)) {
        body_def.bullet = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "active");
    if (!lua_isnil(L, -1)) {
        body_def.active = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    LTBody *body = new LTBody(world, &body_def);
    push_wrap(L, body);
    add_ref(L, 1, -1); // Add reference from world to body.
    add_ref(L, -1, 1); // Add reference from body to world.
    return 1;
}

static void read_common_joint_def_from_table(lua_State *L, int table, b2JointDef *def) {
    def->userData = NULL;

    lua_getfield(L, table, "body1");
    if (lua_isnil(L, -1)) {
        luaL_error(L, "Missing body1 field in revolution joint definition");
    }
    b2Body *body1 = ((LTBody*)get_object(L, -1, LT_TYPE_BODY))->body;
    lua_pop(L, 1);
    if (body1 == NULL) {
        luaL_error(L, "body1 is destroyed");
    }
    def->bodyA = body1;

    lua_getfield(L, table, "body2");
    if (lua_isnil(L, -1)) {
        luaL_error(L, "Missing body2 field in revolution joint definition");
    }
    b2Body *body2 = ((LTBody*)get_object(L, -1, LT_TYPE_BODY))->body;
    lua_pop(L, 1);
    if (body2 == NULL) {
        luaL_error(L, "body2 is destroyed");
    }
    def->bodyB = body2;

    lua_getfield(L, table, "collide");
    if (!lua_isnil(L, -1)) {
        def->collideConnected = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);
}

static void read_revolute_joint_def_from_table(lua_State *L, int table, b2RevoluteJointDef *def) {
    def->type = e_revoluteJoint;
    read_common_joint_def_from_table(L, table, def);

    lua_getfield(L, table, "anchor1");
    if (lua_isnil(L, -1)) {
        luaL_error(L, "Missing anchor1 field in revolute joint definition");
    }
    if (lua_istable(L, -1)) {
        lua_rawgeti(L, -1, 1);
        def->localAnchorA.x = luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, -1, 2);
        def->localAnchorA.y = luaL_checknumber(L, -1);
        lua_pop(L, 1);
    } else {
        luaL_error(L, "Expecting anchor1 field to be a table");
    }
    lua_pop(L, 1);

    lua_getfield(L, table, "anchor2");
    if (lua_isnil(L, -1)) {
        // Use the current world positions of the bodies to
        // compute anchor2 from anchor1.
        b2Vec2 anchor1_w = def->bodyA->GetWorldPoint(def->localAnchorA);
        def->localAnchorB = def->bodyB->GetLocalPoint(anchor1_w);
    } else {
        if (lua_istable(L, -1)) {
            lua_rawgeti(L, -1, 1);
            def->localAnchorB.x = luaL_checknumber(L, -1);
            lua_pop(L, 1);
            lua_rawgeti(L, -1, 2);
            def->localAnchorB.y = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        } else {
            luaL_error(L, "Expecting anchor2 field to be a table");
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, table, "angle");
    if (lua_isnil(L, -1)) {
        // Compute the reference angle from the bodies' current angles.
        def->referenceAngle = def->bodyB->GetAngle() - def->bodyA->GetAngle();
    } else {
        def->referenceAngle = luaL_checknumber(L, -1) * LT_RADIANS_PER_DEGREE;
    }
    lua_pop(L, 1);

    lua_getfield(L, table, "lower_limit");
    if (!lua_isnil(L, -1)) {
        def->lowerAngle = luaL_checknumber(L, -1) * LT_RADIANS_PER_DEGREE;
        def->enableLimit = true;
    }
    lua_pop(L, 1);

    lua_getfield(L, table, "upper_limit");
    if (!lua_isnil(L, -1)) {
        def->upperAngle = luaL_checknumber(L, -1) * LT_RADIANS_PER_DEGREE;
        def->enableLimit = true;
    }
    lua_pop(L, 1);
}

static void read_distance_joint_def_from_table(lua_State *L, int table, b2DistanceJointDef *def) {
    def->type = e_distanceJoint;
    read_common_joint_def_from_table(L, table, def);

    lua_getfield(L, table, "anchor1");
    if (lua_isnil(L, -1)) {
        luaL_error(L, "Missing anchor1 field in distance joint definition");
    }
    if (lua_istable(L, -1)) {
        lua_rawgeti(L, -1, 1);
        def->localAnchorA.x = luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, -1, 2);
        def->localAnchorA.y = luaL_checknumber(L, -1);
        lua_pop(L, 1);
    } else {
        luaL_error(L, "Expecting anchor1 field to be a table");
    }
    lua_pop(L, 1);

    lua_getfield(L, table, "anchor2");
    if (lua_isnil(L, -1)) {
        luaL_error(L, "Missing anchor2 field in distance joint definition");
    } else {
        if (lua_istable(L, -1)) {
            lua_rawgeti(L, -1, 1);
            def->localAnchorB.x = luaL_checknumber(L, -1);
            lua_pop(L, 1);
            lua_rawgeti(L, -1, 2);
            def->localAnchorB.y = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        } else {
            luaL_error(L, "Expecting anchor2 field to be a table");
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, table, "length");
    if (lua_isnil(L, -1)) {
        // Compute the length from the two anchor points.
        b2Vec2 world_anchor1 = def->bodyA->GetWorldPoint(def->localAnchorA);
        b2Vec2 world_anchor2 = def->bodyB->GetWorldPoint(def->localAnchorB);
        b2Vec2 diff = world_anchor2 - world_anchor1;
        def->length = diff.Length();
    } else {
        def->length = luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, table, "frequency");
    if (!lua_isnil(L, -1)) {
        def->frequencyHz = luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, table, "damping");
    if (!lua_isnil(L, -1)) {
        def->dampingRatio = luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);
}

static int lt_AddJointToWorld(lua_State *L) {
    // First argument is world, second is joint definition (a table).
    check_nargs(L, 2);
    LTWorld *world = (LTWorld*)get_object(L, 1, LT_TYPE_WORLD);
    lua_getfield(L, 2, "type");
    const char *joint_type_str = lua_tostring(L, -1);
    lua_pop(L, 1);
    if (joint_type_str == NULL) {
        return luaL_error(L, "Joint type not specified");
    }
    b2JointDef *def = NULL;
    b2RevoluteJointDef rdef;
    b2DistanceJointDef ddef;
    if (strcmp(joint_type_str, "revolute") == 0) {
        read_revolute_joint_def_from_table(L, 2, &rdef);
        def = &rdef;
    } else if (strcmp(joint_type_str, "distance") == 0) {
        read_distance_joint_def_from_table(L, 2, &ddef);
        def = &ddef;
    } else {
        return luaL_error(L, "Unsupported joint type: %s", joint_type_str);
    }
    LTJoint *joint = new LTJoint(world, def);
    push_wrap(L, joint);
    add_ref(L, 1, -1); // Add reference from world to joint.
    add_ref(L, -1, 1); // Add reference from joint to world.
    return 1;
}

static void get_body_and_fixture(lua_State *L, int arg, b2Body **body, b2Fixture **fixture) {
    LTObject *obj = get_object(L, arg, LT_TYPE_OBJECT);
    *body = NULL;
    *fixture = NULL;
    if (obj->type == LT_TYPE_BODY) {
        *body = ((LTBody*)obj)->body;
    } else if (obj->type == LT_TYPE_FIXTURE) {
        *fixture = ((LTFixture*)obj)->fixture;
        if (*fixture != NULL) {
            *body = (*fixture)->GetBody();
        }
    }
}

static int lt_BodyOrFixtureTouching(lua_State *L) {
    int nargs = check_nargs(L, 1);
    b2Body *b1 = NULL;
    b2Body *b2 = NULL;
    b2Fixture *f1 = NULL;
    b2Fixture *f2 = NULL;
    get_body_and_fixture(L, 1, &b1, &f1);
    if (nargs > 1) {
        get_body_and_fixture(L, 2, &b2, &f2);
    }
    if (b1 == NULL) {
        lua_pushboolean(L, 0);
        return 1;
    }
    b2ContactEdge *edge = b1->GetContactList();
    while (edge != NULL) {
        if (b2 == NULL || edge->other == b2) {
            b2Contact *contact = edge->contact;
            if (contact->IsTouching()) {
                if (f1 == NULL && f2 == NULL) {
                    lua_pushboolean(L, 1);
                    return 1;
                } else {
                    b2Fixture *a = contact->GetFixtureA();
                    b2Fixture *b = contact->GetFixtureB();
                    if (
                           (f1 != NULL && f2 == NULL && (a == f1 || b == f1))
                        || (f1 == NULL && f2 != NULL && (a == f2 || b == f2)) 
                        || (f1 != NULL && f2 != NULL && ((a == f1 && b == f2) || (a == f2 && b == f1)))
                    ) {
                        lua_pushboolean(L, 1);
                        return 1;
                    }
                }
            }
        }
        edge = edge->next;
    }
    lua_pushboolean(L, 0);
    return 1;
}

struct RayCastData {
    b2Fixture *fixture;
    b2Vec2 point;
    b2Vec2 normal;
};

struct RayCastCallback : public b2RayCastCallback {
    std::map<LTfloat, RayCastData> hits;

    RayCastCallback() { }

    virtual float32 ReportFixture(b2Fixture* fixture,
        const b2Vec2& point, const b2Vec2& normal, float32 fraction)
    {
        RayCastData data;
        data.fixture = fixture;
        data.point = point;
        data.normal = normal;
        hits[fraction] = data;
        return 1.0f;
    }
};

static int lt_WorldRayCast(lua_State *L) {
    check_nargs(L, 5);
    LTWorld *world = (LTWorld*)get_object(L, 1, LT_TYPE_WORLD);
    LTfloat x1 = luaL_checknumber(L, 2);
    LTfloat y1 = luaL_checknumber(L, 3);
    LTfloat x2 = luaL_checknumber(L, 4);
    LTfloat y2 = luaL_checknumber(L, 5);
    
    RayCastCallback cb;
    world->world->RayCast(&cb, b2Vec2(x1, y1), b2Vec2(x2, y2));

    lua_newtable(L);
    int i = 1;
    std::map<LTfloat, RayCastData>::iterator it;
    for (it = cb.hits.begin(); it != cb.hits.end(); it++) {
        lua_newtable(L);
        push_wrap(L, (LTFixture*)it->second.fixture->GetUserData());
        lua_setfield(L, -2, "fixture");
        lua_pushnumber(L, it->second.point.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, it->second.point.y);
        lua_setfield(L, -2, "y");
        lua_pushnumber(L, it->second.normal.x);
        lua_setfield(L, -2, "normal_x");
        lua_pushnumber(L, it->second.normal.y);
        lua_setfield(L, -2, "normal_y");
        lua_pushnumber(L, it->first);
        lua_setfield(L, -2, "fraction");
        lua_rawseti(L, -2, i);
        i++;
    }
    return 1;
}

static int lt_World(lua_State *L) {
    int nargs = check_nargs(L, 0);
    LTfloat scaling = 1.0f;
    if (nargs > 0) {
        scaling = luaL_checknumber(L, 1);
    }
    LTWorld *world = new LTWorld(b2Vec2(0.0f, -10.0f), true, scaling);
    push_wrap(L, world);
    return 1;
}

static int lt_BodyTracker(lua_State *L) {
    int nargs = check_nargs(L, 2);
    LTSceneNode *child = (LTSceneNode *)get_object(L, 1, LT_TYPE_SCENENODE);
    LTBody *body = (LTBody *)get_object(L, 2, LT_TYPE_BODY);
    bool viewport_mode = false;
    bool track_rotation = true;
    LTfloat min_x = -FLT_MAX;
    LTfloat max_x = FLT_MAX;
    LTfloat min_y = -FLT_MAX;
    LTfloat max_y = FLT_MAX;
    LTfloat snap_to = 0.0f;
    if (nargs > 2) {
        snap_to = luaL_checknumber(L, 3);
    }
    if (nargs > 3) {
        viewport_mode = lua_toboolean(L, 4);
    }
    if (nargs > 4) {
        track_rotation = lua_toboolean(L, 5);
    }
    if (nargs > 5) {
        min_x = luaL_checknumber(L, 6);
    }
    if (nargs > 6) {
        max_x = luaL_checknumber(L, 7);
    }
    if (nargs > 7) {
        min_y = luaL_checknumber(L, 8);
    }
    if (nargs > 8) {
        max_y = luaL_checknumber(L, 9);
    }
    LTBodyTracker *node = new LTBodyTracker(body, child, viewport_mode, track_rotation,
        min_x, max_x, min_y, max_y, snap_to);
    push_wrap(L, node);
    set_ref_field(L, -1, "child", 1); // Add reference from new node to child.
    set_ref_field(L, -1, "body", 2);  // Add reference from new node to body.
    return 1;
}

/********************* Game Center *****************************/

static int lt_GameCenterAvailable(lua_State *L) {
    #ifdef LTGAMECENTER
    lua_pushboolean(L, ltIOSGameCenterIsAvailable());
    #else
    lua_pushboolean(L, 0);
    #endif
    return 1;
}

static int lt_SubmitScore(lua_State *L) {
    check_nargs(L, 2);
    const char *leaderboard = lua_tostring(L, 1);
    if (leaderboard != NULL) {
        #ifdef LTGAMECENTER
        int score = lua_tointeger(L, 2);
        ltIOSSubmitGameCenterScore(score, leaderboard);
        #endif
    }
    return 0;
}

static int lt_ShowLeaderboard(lua_State *L) {
    check_nargs(L, 1);
    const char *leaderboard = lua_tostring(L, 1);
    if (leaderboard != NULL) {
        #ifdef LTGAMECENTER
        ltIOSShowGameCenterLeaderBoard(leaderboard);
        #endif
    } else {
        return luaL_error(L, "Expecting a string argument");
    }
    return 0;
}

/********************* URL launcher *****************************/

static int lt_OpenURL(lua_State *L) {
    check_nargs(L, 1);
    const char *url = lua_tostring(L, 1);
    if (url != NULL) {
        #ifdef LTIOS
        ltIOSLaunchURL(url);
        #endif
    } else {
        return luaL_error(L, "Expecting a string argument");
    }
    return 0;
}

/************************* Random numbers **************************/

static int lt_Random(lua_State *L) {
    check_nargs(L, 1);
    int seed = luaL_checkinteger(L, 1);
    LTRandomGenerator *r = new LTRandomGenerator(seed);
    push_wrap(L, r);
    return 1;
}

static int lt_NextRandomInt(lua_State *L) {
    int nargs = check_nargs(L, 2);
    LTRandomGenerator *r = (LTRandomGenerator*)get_object(L, 1, LT_TYPE_RANDOMGENERATOR);
    if (nargs > 2) {
        int min = luaL_checkinteger(L, 2);
        int max = luaL_checkinteger(L, 3);
        lua_pushinteger(L, r->nextInt(max - min + 1) + min);
    } else {
        int max = luaL_checkinteger(L, 2);
        lua_pushinteger(L, r->nextInt(max) + 1);
    }
    return 1;
}

static int lt_NextRandomNumber(lua_State *L) {
    int nargs = check_nargs(L, 1);
    LTRandomGenerator *r = (LTRandomGenerator*)get_object(L, 1, LT_TYPE_RANDOMGENERATOR);
    if (nargs == 1) {
        lua_pushnumber(L, r->nextDouble());
    }
    if (nargs == 2) {
        LTdouble scale = luaL_checknumber(L, 2);
        lua_pushnumber(L, r->nextDouble() * scale);
    } else {
        LTdouble min = luaL_checknumber(L, 2);
        LTdouble max = luaL_checknumber(L, 3);
        lua_pushnumber(L, min + (max - min) * r->nextDouble());
    }
    return 1;
}

static int lt_NextRandomBool(lua_State *L) {
    check_nargs(L, 1);
    LTRandomGenerator *r = (LTRandomGenerator*)get_object(L, 1, LT_TYPE_RANDOMGENERATOR);
    lua_pushboolean(L, r->nextBool());
    return 1;
}

/********************* Loading *****************************/

/*
 * These come from lauxlib.c.  We have modified luaL_loadfile
 * to use only the base file name as the chunk name.
 */

typedef struct LoadF {
    LTResource *r;
    char buff[LUAL_BUFFERSIZE];
    int eof;
} LoadF;


static const char *getF (lua_State *L, void *ud, size_t *size) {
    LoadF *lf = (LoadF *)ud;
    if (lf->eof) {
        return NULL;
    }
    *size = ltReadResource(lf->r, lf->buff, sizeof(lf->buff));
    if (*size < sizeof(lf->buff)) {
        lf->eof = 1;
    }
    return (*size > 0) ? lf->buff : NULL;
}


static int loadfile (lua_State *L, LTResource *rsc) {
  LoadF lf;
  int status;
  int fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */

  const char *basename = strrchr(rsc->name, '/');
  if (basename == NULL) {
    basename = rsc->name;
  } else {
    basename++;
  }

  lua_pushfstring(L, "@%s", basename);
  lf.r = rsc;
  lf.eof = 0;
  status = lua_load(L, getF, &lf, lua_tostring(L, -1));
  lua_remove(L, fnameindex);
  return status;
}

static int import(lua_State *L) {
    int top = lua_gettop(L);
    int nargs = check_nargs(L, 1);
    const char *module = lua_tostring(L, 1);
    if (module == NULL) {
        return luaL_error(L, "The import function requires a string argument.");
    }
    const char *path;
    path = resource_path(module, ".lua");
    LTResource *rsc = ltOpenResource(path);
    if (rsc != NULL) {
        int r = loadfile(g_L, rsc);
        delete[] path;
        ltCloseResource(rsc);
        if (r != 0) {
            const char *msg = lua_tostring(g_L, -1);
            lua_pop(L, 1);
            return luaL_error(L, "%s", msg);
        }
        for (int i = 2; i <= nargs; i++) {
            lua_pushvalue(L, i);
        }
        lua_call(g_L, nargs - 1, LUA_MULTRET);
        return lua_gettop(L) - top;
    } else {
        return luaL_error(L, "File %s does no exist", path);
    }
}

/************************ Logging *****************************/

static int log(lua_State *L) {
    check_nargs(L, 1);
    lua_Debug ar;
    lua_getstack(L, 1, &ar);
    lua_getinfo(L, "nSl", &ar);
    int line = ar.currentline;
    const char *source = ar.source;
    const char *filename = "<<string>>";
    if (source[0] == '@') {
        filename = &source[1];
    }
    const char *msg = lua_tostring(L, 1);
    if (msg != NULL) {
        ltLog("%s:%d: %s", filename, line, msg);
    } else {
        ltLog("Unable to log NULL message");
    }
    return 0;
}

/************************************************************/

static const luaL_Reg ltlib[] = {
    {"GetObjectField",                  lt_GetObjectField},
    {"SetObjectField",                  lt_SetObjectField},

    {"SetViewPort",                     lt_SetViewPort},
    {"SetDesignScreenSize",             lt_SetDesignScreenSize},
    {"SetOrientation",                  lt_SetOrientation},
    {"PushTint",                        lt_PushTint},
    {"PopTint",                         lt_PopTint},
    {"PushMatrix",                      lt_PushMatrix},
    {"PopMatrix",                       lt_PopMatrix},
    {"DrawUnitSquare",                  lt_DrawUnitSquare},
    {"DrawUnitCircle",                  lt_DrawUnitCircle},
    {"DrawRect",                        lt_DrawRect},
    {"DrawEllipse",                     lt_DrawEllipse},

    {"Layer",                           lt_Layer},
    {"Line",                            lt_Line},
    {"Triangle",                        lt_Triangle},
    {"Rect",                            lt_Rect},
    {"Tint",                            lt_Tint},
    {"BlendMode",                       lt_BlendMode},
    {"TextureMode",                     lt_TextureMode},
    {"Scale",                           lt_Scale},
    {"Perspective",                     lt_Perspective},
    {"Pitch",                           lt_Pitch},
    {"Fog",                             lt_Fog},
    {"DepthTest",                       lt_DepthTest},
    {"DepthMask",                       lt_DepthMask},
    {"Translate",                       lt_Translate},
    {"Rotate",                          lt_Rotate},
    {"HitFilter",                       lt_HitFilter},
    {"DownFilter",                      lt_DownFilter},
    {"Wrap",                            lt_Wrap},

    {"DrawSceneNode",                   lt_DrawSceneNode},
    {"AddOnPointerUpHandler",           lt_AddOnPointerUpHandler},
    {"AddOnPointerDownHandler",         lt_AddOnPointerDownHandler},
    {"AddOnPointerMoveHandler",         lt_AddOnPointerMoveHandler},
    {"AddOnPointerOverHandler",         lt_AddOnPointerOverHandler},
    {"PropogatePointerUpEvent",         lt_PropogatePointerUpEvent},
    {"PropogatePointerDownEvent",       lt_PropogatePointerDownEvent},
    {"PropogatePointerMoveEvent",       lt_PropogatePointerMoveEvent},
    {"InsertLayerFront",                lt_InsertLayerFront},
    {"InsertLayerBack",                 lt_InsertLayerBack},
    {"InsertLayerAbove",                lt_InsertLayerAbove},
    {"InsertLayerBelow",                lt_InsertLayerBelow},
    {"RemoveFromLayer",                 lt_RemoveFromLayer},
    {"LayerSize",                       lt_LayerSize},
    {"ReplaceWrappedChild",             lt_ReplaceWrappedChild},

    {"LoadImages",                      lt_LoadImages},

    {"Vector",                          lt_Vector},
    {"GenerateVectorColumn",            lt_GenerateVectorColumn},
    {"FillVectorColumnsWithImageQuads", lt_FillVectorColumnsWithImageQuads},
    {"DrawQuads",                       lt_DrawQuads},
    {"DrawVector",                      lt_DrawVector},

    {"ParticleSystem",                  lt_ParticleSystem},
    {"ParticleSystemAdvance",           lt_ParticleSystemAdvance},
    {"ParticleSystemFixtureFilter",     lt_ParticleSystemFixtureFilter},

    {"RenderTarget",                    lt_RenderTarget},

    {"MakeNativeTween",                 lt_MakeNativeTween},
    {"AdvanceNativeTween",              lt_AdvanceNativeTween},
    //{"TweenSet",                        lt_TweenSet},
    //{"AddTween",                        lt_AddTween},
    //{"AdvanceTweens",                   lt_AdvanceTweens},
    //{"ClearTweens",                     lt_ClearTweens},

    {"LoadSamples",                     lt_LoadSamples},
    {"Track",                           lt_Track},
    {"PlaySampleOnce",                  lt_PlaySampleOnce},
    {"PlayTrack",                       lt_PlayTrack},
    {"PauseTrack",                      lt_PauseTrack},
    {"StopTrack",                       lt_StopTrack},
    {"RewindTrack",                     lt_RewindTrack},
    {"QueueSampleInTrack",              lt_QueueSampleInTrack},
    {"SetTrackLoop",                    lt_SetTrackLoop},
    {"TrackQueueSize",                  lt_TrackQueueSize},
    {"TrackNumPlayed",                  lt_TrackNumPlayed},
    {"TrackNumPending",                 lt_TrackNumPending},
    {"TrackDequeuePlayed",              lt_TrackDequeuePlayed},
    {"SampleNumDataPoints",             lt_SampleNumDataPoints},
    {"SampleFrequency",                 lt_SampleFrequency},
    {"SampleLength",                    lt_SampleLength},
    
    /*
    {"Store",                           lt_Store},
    {"Retrieve",                        lt_Retrieve},
    */

    {"World",                           lt_World},
    {"FixtureContainsPoint",            lt_FixtureContainsPoint},
    {"DestroyFixture",                  lt_DestroyFixture},
    {"FixtureIsDestroyed",              lt_FixtureIsDestroyed},
    {"DoWorldStep",                     lt_DoWorldStep},
    {"SetWorldGravity",                 lt_SetWorldGravity},
    {"SetWorldAutoClearForces",         lt_SetWorldAutoClearForces},
    {"WorldQueryBox",                   lt_WorldQueryBox},
    {"DestroyBody",                     lt_DestroyBody},
    {"BodyIsDestroyed",                 lt_BodyIsDestroyed},
    {"ApplyForceToBody",                lt_ApplyForceToBody},
    {"ApplyTorqueToBody",               lt_ApplyTorqueToBody},
    {"ApplyImpulseToBody",              lt_ApplyImpulseToBody},
    {"ApplyAngularImpulseToBody",       lt_ApplyAngularImpulseToBody},
    //{"ClearBodyForces",                 lt_ClearBodyForces},
    {"GetBodyAngle",                    lt_GetBodyAngle},
    {"SetBodyAngle",                    lt_SetBodyAngle},
    {"GetBodyPosition" ,                lt_GetBodyPosition},
    {"SetBodyPosition" ,                lt_SetBodyPosition},
    {"GetBodyVelocity" ,                lt_GetBodyVelocity},
    {"SetBodyVelocity" ,                lt_SetBodyVelocity},
    {"SetBodyAngularVelocity",          lt_SetBodyAngularVelocity},
    {"SetBodyGravityScale",             lt_SetBodyGravityScale},
    {"AddRectToBody",                   lt_AddRectToBody},
    {"AddTriangleToBody",               lt_AddTriangleToBody},
    {"AddPolygonToBody",                lt_AddPolygonToBody},
    {"AddCircleToBody",                 lt_AddCircleToBody},
    {"GetFixtureBody",                  lt_GetFixtureBody},
    {"GetBodyFixtures",                 lt_GetBodyFixtures},
    {"FixtureBoundingBox",              lt_FixtureBoundingBox},
    {"AddStaticBodyToWorld",            lt_AddStaticBodyToWorld},
    {"AddDynamicBodyToWorld",           lt_AddDynamicBodyToWorld},
    {"AddBodyToWorld",                  lt_AddBodyToWorld},
    {"AddJointToWorld",                 lt_AddJointToWorld},
    {"BodyOrFixtureTouching",           lt_BodyOrFixtureTouching},
    {"BodyTracker",                     lt_BodyTracker},
    {"WorldRayCast",                    lt_WorldRayCast},

    {"GameCenterAvailable",             lt_GameCenterAvailable},
    {"SubmitScore",                     lt_SubmitScore},
    {"ShowLeaderboard",                 lt_ShowLeaderboard},

    {"OpenURL",                         lt_OpenURL},

    {"Random",                          lt_Random},
    {"NextRandomInt",                   lt_NextRandomInt},
    {"NextRandomNumber",                lt_NextRandomNumber},
    {"NextRandomBool",                  lt_NextRandomBool},

    {NULL, NULL}
};

/************************************************************/

static bool push_lt_func(const char *func) {
    lua_getglobal(g_L, "lt");
    if (lua_istable(g_L, -1)) {
        lua_getfield(g_L, -1, func);
        lua_remove(g_L, -2); // Remove lt table.
        if (lua_isfunction(g_L, -1)) {
            return true;
        } else {
            lua_pop(g_L, 1); // Pop the field since we won't be calling it.
            return false;
        }
    } else {
        lua_pop(g_L, 1);
        return false;
    }
}

static void call_lt_func(const char *func) {
    if (push_lt_func(func)) {
        docall(g_L, 0);
    }
}

static void run_lua_file(const char *file) {
    if (!g_suspended) {
        const char *f = resource_path(file, ".lua");
        LTResource *r = ltOpenResource(f);
        if (r != NULL) {
            check_status(loadfile(g_L, r));
            docall(g_L, 0);
            ltCloseResource(r);
        } else {
            ltLog("File %s does not exist", f);
        }
        delete[] f;
    }
}

static void setup_userdata_key_ref() {
    lua_pushstring(g_L, LT_USERDATA_KEY);
    g_userdata_key_ref = luaL_ref(g_L, LUA_REGISTRYINDEX);
}

static void setup_wref_ref() {
    lua_getglobal(g_L, "lt");
    lua_getfield(g_L, -1, "wrefs");
    g_wrefs_ref = luaL_ref(g_L, LUA_REGISTRYINDEX);
    lua_pop(g_L, 1); // pop lt.
}

static void setup_string_table_ref() {
    lua_newtable(g_L);
    g_string_table_ref = luaL_ref(g_L, LUA_REGISTRYINDEX);
}

static void setup_ud_metatables_ref() {
    lua_getglobal(g_L, "lt");
    lua_getfield(g_L, -1, "metatables");
    g_ud_metatables_ref = luaL_ref(g_L, LUA_REGISTRYINDEX);
    lua_pop(g_L, 1); // pop lt.
}

static void set_viewport_globals() {
    if (g_L != NULL) {
        lua_getglobal(g_L, "lt");
        lua_pushnumber(g_L, ltGetViewPortLeftEdge());
        lua_setfield(g_L, -2, "left");
        lua_pushnumber(g_L, ltGetViewPortBottomEdge());
        lua_setfield(g_L, -2, "bottom");
        lua_pushnumber(g_L, ltGetViewPortRightEdge());
        lua_setfield(g_L, -2, "right");
        lua_pushnumber(g_L, ltGetViewPortTopEdge());
        lua_setfield(g_L, -2, "top");
        lua_pushnumber(g_L, ltGetViewPortRightEdge() - ltGetViewPortLeftEdge());
        lua_setfield(g_L, -2, "width");
        lua_pushnumber(g_L, ltGetViewPortTopEdge() - ltGetViewPortBottomEdge());
        lua_setfield(g_L, -2, "height");
        lua_pop(g_L, 1); // pop lt
    }
}

static void set_globals() {
    if (g_L != NULL) {
        lua_getglobal(g_L, "lt");
        #ifdef LTIOS
            lua_pushboolean(g_L, 1);
            lua_setfield(g_L, -2, "ios");
            lua_pushboolean(g_L, ltIsIPad() ? 1 : 0);
            lua_setfield(g_L, -2, "ipad");
            lua_pushboolean(g_L, ltIOSSupportsES2());
            lua_setfield(g_L, -2, "shaders");
            #ifdef LTADS
                if (LTADS == LT_AD_TOP) {
                    lua_pushstring(g_L, "top");
                } else {
                    lua_pushstring(g_L, "bottom");
                }
                lua_setfield(g_L, -2, "ads");
            #endif
        #endif
        #ifdef LTOSX
            lua_pushboolean(g_L, 1);
            lua_setfield(g_L, -2, "osx");
            lua_pushboolean(g_L, 1);
            lua_setfield(g_L, -2, "desktop");
            lua_pushboolean(g_L, 1);
            lua_setfield(g_L, -2, "shaders");
        #endif
        lua_pop(g_L, 1); // pop lt
    }
}

void ltLuaSetup() {
    ltInitObjectFieldCache();
    #ifndef LTANDROID
    ltAudioInit();
    #endif
    g_L = luaL_newstate();
    if (g_L == NULL) {
        ltLog("Cannot create lua state: not enough memory.");
        ltAbort();
    }
    lua_gc(g_L, LUA_GCSTOP, 0);  /* stop collector during library initialization */
    luaL_openlibs(g_L);
    lua_pushcfunction(g_L, import);
    lua_setglobal(g_L, "import");
    lua_pushcfunction(g_L, log);
    lua_setglobal(g_L, "log");
    luaL_register(g_L, "lt", ltlib);
    luaL_register(g_L, "lt", ltlib);
    lua_gc(g_L, LUA_GCRESTART, 0);
    run_lua_file("lt");
    setup_userdata_key_ref();
    setup_wref_ref();
    setup_ud_metatables_ref();
    setup_string_table_ref();
    set_globals();
    run_lua_file("config");
    ltRestoreState();
}

void ltLuaSetResourcePrefix(const char *prefix) {
    resource_prefix = prefix;
}

void ltLuaTeardown() {
    if (g_L != NULL) {
        lua_close(g_L);
        g_L = NULL;
    }
    ltAudioTeardown();
}

void ltLuaReset() {
    ltSaveState();
    ltLuaTeardown();
    g_suspended = false;
    g_initialized = false;
    g_gamecenter_initialized = false;
    ltLuaSetup();
}

void ltLuaSuspend() {
    g_suspended = true;
    ltAudioSuspend();
}

void ltLuaResume() {
    g_suspended = false;
    ltAudioResume();
}

void ltLuaAdvance(LTdouble secs) {
    if (g_L != NULL && !g_suspended && push_lt_func("Advance")) {
        lua_pushnumber(g_L, secs);
        docall(g_L, 1);
    }
    ltAudioGC();
}

void ltLuaRender() {
    if (g_L != NULL && !g_suspended) {
        if (!g_initialized) {
            ltInitGLState();
            ltAdjustViewportAspectRatio();
            set_viewport_globals();
            run_lua_file("main");
            #ifdef LTGAMECENTER
            if (ltIOSGameCenterIsAvailable()) {
                ltLuaGameCenterBecameAvailable();
            }
            #endif
            g_initialized = true;
        }
        if (!g_suspended) {
            ltInitGraphics();
            call_lt_func("Render");
            ltDrawAdBackground();
            ltDrawConnectingOverlay();
        }
    }
}

static const char *lt_key_str(LTKey key) {
    switch (key) {
        case LT_KEY_0: return "0"; 
        case LT_KEY_1: return "1"; 
        case LT_KEY_2: return "2"; 
        case LT_KEY_3: return "3"; 
        case LT_KEY_4: return "4"; 
        case LT_KEY_5: return "5"; 
        case LT_KEY_6: return "6"; 
        case LT_KEY_7: return "7"; 
        case LT_KEY_8: return "8"; 
        case LT_KEY_9: return "9"; 
        case LT_KEY_A: return "A"; 
        case LT_KEY_B: return "B"; 
        case LT_KEY_C: return "C"; 
        case LT_KEY_D: return "D"; 
        case LT_KEY_E: return "E"; 
        case LT_KEY_F: return "F"; 
        case LT_KEY_G: return "G"; 
        case LT_KEY_H: return "H"; 
        case LT_KEY_I: return "I"; 
        case LT_KEY_J: return "J"; 
        case LT_KEY_K: return "K"; 
        case LT_KEY_L: return "L"; 
        case LT_KEY_M: return "M"; 
        case LT_KEY_N: return "N"; 
        case LT_KEY_O: return "O"; 
        case LT_KEY_P: return "P"; 
        case LT_KEY_Q: return "Q"; 
        case LT_KEY_R: return "R"; 
        case LT_KEY_S: return "S"; 
        case LT_KEY_T: return "T"; 
        case LT_KEY_U: return "U"; 
        case LT_KEY_V: return "V"; 
        case LT_KEY_W: return "W"; 
        case LT_KEY_X: return "X"; 
        case LT_KEY_Y: return "Y"; 
        case LT_KEY_Z: return "Z"; 
        case LT_KEY_SPACE: return "space"; 
        case LT_KEY_TAB: return "tab"; 
        case LT_KEY_ENTER: return "enter"; 
        case LT_KEY_UP: return "up"; 
        case LT_KEY_DOWN: return "down"; 
        case LT_KEY_LEFT: return "left"; 
        case LT_KEY_RIGHT: return "right"; 
        case LT_KEY_RIGHT_BRACKET: return "["; 
        case LT_KEY_LEFT_BRACKET: return "]"; 
        case LT_KEY_BACKSLASH: return "\\"; 
        case LT_KEY_SEMI_COLON: return ":"; 
        case LT_KEY_APOS: return ";"; 
        case LT_KEY_COMMA: return ","; 
        case LT_KEY_PERIOD: return "."; 
        case LT_KEY_SLASH: return "/"; 
        case LT_KEY_PLUS: return "+"; 
        case LT_KEY_MINUS: return "-"; 
        case LT_KEY_TICK: return "`"; 
        case LT_KEY_DEL: return "del"; 
        case LT_KEY_ESC: return "esc"; 
        case LT_KEY_UNKNOWN: return "unknown";
    }
    return "";
}

void ltLuaKeyDown(LTKey key) {
    if (g_L != NULL && !g_suspended && push_lt_func("KeyDown")) {
        const char *str = lt_key_str(key);
        lua_pushstring(g_L, str);
        docall(g_L, 1);
    }
}

void ltLuaKeyUp(LTKey key) {
    if (g_L != NULL && !g_suspended && push_lt_func("KeyUp")) {
        const char *str = lt_key_str(key);
        lua_pushstring(g_L, str);
        docall(g_L, 1);
    }
}

void ltLuaPointerDown(int input_id, LTfloat x, LTfloat y) {
    if (g_L != NULL && !g_suspended && push_lt_func("PointerDown")) {
        lua_pushinteger(g_L, input_id);
        lua_pushnumber(g_L, ltGetViewPortX(x));
        lua_pushnumber(g_L, ltGetViewPortY(y));
        docall(g_L, 3);
    }
}

void ltLuaPointerUp(int input_id, LTfloat x, LTfloat y) {
    if (g_L != NULL && !g_suspended && push_lt_func("PointerUp")) {
        lua_pushinteger(g_L, input_id);
        lua_pushnumber(g_L, ltGetViewPortX(x));
        lua_pushnumber(g_L, ltGetViewPortY(y));
        docall(g_L, 3);
    }
}

void ltLuaPointerMove(int input_id, LTfloat x, LTfloat y) {
    if (g_L != NULL && !g_suspended && push_lt_func("PointerMove")) {
        lua_pushinteger(g_L, input_id);
        lua_pushnumber(g_L, ltGetViewPortX(x));
        lua_pushnumber(g_L, ltGetViewPortY(y));
        docall(g_L, 3);
    }
}

void ltLuaResizeWindow(LTfloat w, LTfloat h) {
    ltResizeScreen((int)w, (int)h);
    if (g_initialized) {
        ltAdjustViewportAspectRatio();
    }
}

/************************************************************/

void ltLuaGameCenterBecameAvailable() {
    if (!g_gamecenter_initialized &&
        g_L != NULL && !g_suspended &&
        push_lt_func("GameCenterBecameAvailable"))
    {
        docall(g_L, 0);
        g_gamecenter_initialized = true;
    }
}

/************************************************************/

void ltLuaGarbageCollect() {
    if (g_L != NULL) {
        lua_gc(g_L, LUA_GCCOLLECT, 0);
        lua_gc(g_L, LUA_GCCOLLECT, 0);
    }
}

/************************************************************/

#define LT_LUA_TNIL     0
#define LT_LUA_TNUMBER  1
#define LT_LUA_TBOOLEAN 2
#define LT_LUA_TSTRING  3
#define LT_LUA_TTABLE   4

// Pickle the value at the top of the stack.
// Does not alter the stack.
static void pickle_value(lua_State *L, LTPickler *pickler) {
    int ltype = lua_type(L, -1);
    switch (ltype) {
        case LUA_TNIL: {
            pickler->writeByte(LT_LUA_TNIL);
            break;
        }
        case LUA_TNUMBER: {
            LTdouble d = lua_tonumber(L, -1);
            pickler->writeByte(LT_LUA_TNUMBER);
            pickler->writeDouble(d);
            break;
        }
        case LUA_TBOOLEAN: {
            bool b = lua_toboolean(L, -1) == 1 ? true : false;
            pickler->writeByte(LT_LUA_TBOOLEAN);
            pickler->writeBool(b);
            break;
        }
        case LUA_TSTRING: {
            const char *s = lua_tostring(L, -1);
            pickler->writeByte(LT_LUA_TSTRING);
            pickler->writeString(s);
            break;
        }
        case LUA_TTABLE: {
            pickler->writeByte(LT_LUA_TTABLE);
            lua_pushnil(L);
            while (lua_next(L, -2) != 0) {
                lua_pushvalue(L, -2); // Push key.
                pickle_value(L, pickler); // Pickle key.
                lua_pop(L, 1); // Pop key.
                pickle_value(L, pickler); // Pickle value.
                lua_pop(L, 1); // Pop value.
            }
            // Write nil as end-of-table-data marker.
            pickler->writeByte(LT_LUA_TNIL);
            break;
        }
        default: {
            ltLog("Error: Unsupported pickle type: %d", ltype);
            pickler->writeByte(LT_LUA_TNIL);
        }
    }
}

/*
static void dump_fields(lua_State *L) {
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        lua_pop(L, 1); // Pop value.
        fprintf(stderr, "FIELD = %s\n", lua_tostring(L, -1));
    }
}
*/

LTPickler *ltLuaPickleState() {
    if (g_L != NULL) {
        lua_getglobal(g_L, "lt");
        lua_getfield(g_L, -1, "state");
        LTPickler *pickler = new LTPickler();
        pickle_value(g_L, pickler);
        lua_pop(g_L, 2);
        return pickler;
    } else {
        return NULL;
    }
}

// Unpickles a value from the unpickler and pushes it onto
// the stack.
static void unpickle_value(lua_State *L, LTUnpickler *unpickler) {
    unsigned char type = unpickler->readByte();
    switch (type) {
        case LT_LUA_TNIL: {
            lua_pushnil(L);
            break;
        }
        case LT_LUA_TNUMBER: {
            lua_pushnumber(L, unpickler->readDouble());
            break;
        }
        case LT_LUA_TBOOLEAN: {
            lua_pushboolean(L, unpickler->readBool() ? 1 : 0);
            break;
        }
        case LT_LUA_TSTRING: {
            const char *str = unpickler->readString();
            lua_pushstring(L, str);
            delete[] str;
            break;
        }
        case LT_LUA_TTABLE: {
            lua_newtable(L);
            while (true) {
                unpickle_value(L, unpickler); // Unpickle key
                if (lua_isnil(L, -1)) {
                    // A nil key marks the end of the table data.
                    lua_pop(L, 1);
                    break;
                }
                unpickle_value(L, unpickler); // Unpickle value
                lua_settable(L, -3);
            }
            break;
        }
        default: {
            ltLog("Error: Unexpected type while unpickling: %d", type);
            lua_pushnil(L);
        }
    }
}

void ltLuaUnpickleState(LTUnpickler *unpickler) {
    if (g_L != NULL && unpickler != NULL) {
        lua_getglobal(g_L, "lt");
        unpickle_value(g_L, unpickler);
        if (lua_isnil(g_L, -1)) {
            lua_pop(g_L, 1);
            lua_newtable(g_L);
        }
        lua_setfield(g_L, -2, "state");
        lua_pop(g_L, 1);
    }
}

/************************************************************/

const char *ltLuaCacheString(const char *str) {
    if (g_L != NULL) {
        lua_rawgeti(g_L, LUA_REGISTRYINDEX, g_string_table_ref);
        lua_pushstring(g_L, str);
        const char *lstr = lua_tostring(g_L, -1);
        lua_pushboolean(g_L, 1);
        lua_rawset(g_L, -3);
        lua_pop(g_L, 1); // pop string table.
        //ltLog("ltLuaCacheString(\"%s\") = \"%s\"", str, lstr);
        return lstr;
    } else {
        ltLog("Unable to cache string '%s', because the Lua engine has not been initialized.", str);
        ltAbort();
        return NULL;
    }
}

/************************************************************/

void ltLuaPreContextChange() {
    if (g_L == NULL) {
        return;
    }
    lua_State *L = g_L;
    // Traverse weak refs table, calling preContextChange method on 
    // all scene nodes.
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_wrefs_ref);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        if (lua_istable(g_L, -1)) {
            lua_rawgeti(L, LUA_REGISTRYINDEX, g_userdata_key_ref);
            void *ud = lua_touserdata(L, -1);
            if (ud != NULL) {
                LTObject *obj = (LTObject*)ud;
                if (obj->hasType(LT_TYPE_SCENENODE)) {
                    LTSceneNode *node = (LTSceneNode*)obj;
                    node->preContextChange();
                }
            }
            lua_pop(L, 1); // pop userdata
        }
        lua_pop(L, 1); // pop value (key now on top of stack).
    }
    lua_pop(L, 1); // pop wrefs table.
}

void ltLuaPostContextChange() {
    if (g_L == NULL) {
        return;
    }
    lua_State *L = g_L;
    // Traverse weak refs table, calling preContextChange method on 
    // all scene nodes.
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_wrefs_ref);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        if (lua_istable(g_L, -1)) {
            lua_rawgeti(L, LUA_REGISTRYINDEX, g_userdata_key_ref);
            void *ud = lua_touserdata(L, -1);
            if (ud != NULL) {
                LTObject *obj = (LTObject*)ud;
                if (obj->hasType(LT_TYPE_SCENENODE)) {
                    LTSceneNode *node = (LTSceneNode*)obj;
                    node->postContextChange();
                }
            }
            lua_pop(L, 1); // pop userdata
        }
        lua_pop(L, 1); // pop value (key now on top of stack).
    }
    lua_pop(L, 1); // pop wrefs table.
}
