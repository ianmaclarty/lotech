/* Copyright (C) 2010, 2011 Ian MacLarty */

#include "lt.h"

LT_INIT_IMPL(ltlua)

static inline int absidx(lua_State *L, int index) {
    if (index < 0) {
        return lua_gettop(L) + index + 1;
    } else {
        return index;
    }
}

static lua_State *g_L = NULL;
static int g_wrefs_ref = LUA_NOREF;
static bool g_suspended = false;
static bool g_initialized = false;
static bool g_gamecenter_initialized = false;

/************************* Functions for calling lua **************************/

// Check lua_pcall return status.
static void check_status(lua_State *L, int status) {
    if (status) {
        const char *msg = lua_tostring(L, -1);
        lua_pop(L, 1);
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
static void docall(lua_State *L, int nargs, int nresults) {
  int status;
#ifdef LTDEVMODE
  int base = lua_gettop(L) - nargs;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  status = lua_pcall(L, nargs, nresults, base);
  lua_remove(L, base);  /* remove traceback function */
#else
  status = lua_pcall(L, nargs, nresults, 0);
#endif
  check_status(L, status);
}

/************************* Weak references **************************/

// Returns a weak reference to the value at the given index.  Does not
// modify the stack.
static int make_weak_ref(lua_State *L, int index) {
    index = absidx(L, index);
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_wrefs_ref);
    lua_pushvalue(L, index);
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

// Remove a weak reference.
static void del_weak_ref(lua_State *L, int ref) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_wrefs_ref);
    luaL_unref(L, -1, ref);
    lua_pop(L, 1); // pop wrefs
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

static const char *model_path(const char *name) {
    return resource_path(name, ".obj");
}

/************************* Graphics **************************/

static int lt_SetViewPort(lua_State *L) {
    ltLuaCheckNArgs(L, 4);
    LTfloat viewport_x1 = (LTfloat)luaL_checknumber(L, 1);
    LTfloat viewport_y1 = (LTfloat)luaL_checknumber(L, 2);
    LTfloat viewport_x2 = (LTfloat)luaL_checknumber(L, 3);
    LTfloat viewport_y2 = (LTfloat)luaL_checknumber(L, 4);
    ltSetViewPort(viewport_x1, viewport_y1, viewport_x2, viewport_y2);
    return 0;
}

static int lt_SetDesignScreenSize(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTfloat w = (LTfloat)luaL_checknumber(L, 1);
    LTfloat h = (LTfloat)luaL_checknumber(L, 2);
    ltSetDesignScreenSize(w, h);
    return 0;
}

static int lt_SetOrientation(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
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
    int num_args = ltLuaCheckNArgs(L, 3);
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
    ltLuaCheckNArgs(L, 4);
    LTfloat x1 = (LTfloat)luaL_checknumber(L, 1);
    LTfloat y1 = (LTfloat)luaL_checknumber(L, 2);
    LTfloat x2 = (LTfloat)luaL_checknumber(L, 3);
    LTfloat y2 = (LTfloat)luaL_checknumber(L, 4);
    ltDrawRect(x1, y1, x2, y2);
    return 0;
}

static int lt_DrawEllipse(lua_State *L) {
    ltLuaCheckNArgs(L, 4);
    LTfloat x = (LTfloat)luaL_checknumber(L, 1);
    LTfloat y = (LTfloat)luaL_checknumber(L, 2);
    LTfloat rx = (LTfloat)luaL_checknumber(L, 3);
    LTfloat ry = (LTfloat)luaL_checknumber(L, 4);
    ltDrawEllipse(x, y, rx, ry);
    return 0;
}

static int lt_DrawSceneNode(lua_State *L) {
    int nargs = ltLuaCheckNArgs(L, 1);
    LTSceneNode *node = lt_expect_LTSceneNode(L, 1);
    if (nargs > 1) {
        LTRenderTarget *target = lt_expect_LTRenderTarget(L, 2);
        LTColor clear_color(0.0f, 0.0f, 0.0f, 0.0f);
        bool do_clear = false;
        if (nargs > 2) {
            do_clear = true;
            clear_color.red = luaL_checknumber(L, 3);
        }
        if (nargs > 3) {
            clear_color.green = luaL_checknumber(L, 4);
        }
        if (nargs > 4) {
            clear_color.blue = luaL_checknumber(L, 5);
        }
        if (nargs > 5) {
            clear_color.alpha = luaL_checknumber(L, 6);
        }
        target->renderNode(node, do_clear ? &clear_color : NULL);
    } else {
        node->draw();
    }
    return 0;
}

static int lt_InsertLayerFront(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTLayer *layer = lt_expect_LTLayer(L, 1);
    LTSceneNode *node = lt_expect_LTSceneNode(L, 2);
    int ref = ltLuaAddRef(L, 1, 2);
    layer->insert_front(node, ref);
    return 0;
}

static int lt_InsertLayerBack(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTLayer *layer = lt_expect_LTLayer(L, 1);
    LTSceneNode *node = lt_expect_LTSceneNode(L, 2);
    int ref = ltLuaAddRef(L, 1, 2);
    layer->insert_back(node, ref);
    return 0;
}

static int lt_InsertLayerAbove(lua_State *L) {
    ltLuaCheckNArgs(L, 3);
    LTLayer *layer = lt_expect_LTLayer(L, 1);
    LTSceneNode *existing_node = lt_expect_LTSceneNode(L, 2);
    LTSceneNode *new_node = lt_expect_LTSceneNode(L, 3);
    int ref = ltLuaAddRef(L, 1, 3);
    if (!layer->insert_above(existing_node, new_node, ref)) {
        return luaL_error(L, "existing node not in layer");
    }
    return 0;
}

static int lt_InsertLayerBelow(lua_State *L) {
    ltLuaCheckNArgs(L, 3);
    LTLayer *layer = lt_expect_LTLayer(L, 1);
    LTSceneNode *existing_node = lt_expect_LTSceneNode(L, 2);
    LTSceneNode *new_node = lt_expect_LTSceneNode(L, 3);
    int ref = ltLuaAddRef(L, 1, 3);
    if (!layer->insert_below(existing_node, new_node, ref)) {
        return luaL_error(L, "existing node not in layer");
    }
    return 0;
}

static int lt_LayerSize(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTLayer *layer = lt_expect_LTLayer(L, 1);
    lua_pushinteger(L, layer->size());
    return 1;
}

static int lt_RemoveFromLayer(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTLayer *layer = lt_expect_LTLayer(L, 1);
    LTSceneNode *node = lt_expect_LTSceneNode(L, 2);
    layer->remove(L, 1, node);
    return 0;
}

/************************* Vectors **************************/

static int lt_Vector(lua_State *L) {
    int num_args = ltLuaCheckNArgs(L, 1);
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
        vec = new (lt_alloc_LTVector(L)) LTVector(capacity, stride);
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
        vec = new (lt_alloc_LTVector(L)) LTVector(capacity, stride);
        vec->size = capacity;
    } else {
        return luaL_error(L, "Invalid arguments");
    }

    return 1;
}

static int lt_GenerateVectorColumn(lua_State *L) {
    int num_args = ltLuaCheckNArgs(L, 3);
    LTVector *v = lt_expect_LTVector(L, 1);
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
    ltLuaCheckNArgs(L, 5);
    LTVector *vector = lt_expect_LTVector(L, 1);
    int col = luaL_checkinteger(L, 2) - 1;
    LTTexturedNode *img = lt_expect_LTTexturedNode(L, 3);
    LTVector *offsets = lt_expect_LTVector(L, 4);
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

/************************* Events **************************/

struct LTLuaEventHandler : LTEventHandler {
    int func_ref;
    int node_ref;

    LTLuaEventHandler(int node_ref, int func_ref, int filter) : LTEventHandler(filter) {
        LTLuaEventHandler::node_ref = node_ref;
        LTLuaEventHandler::func_ref = func_ref;
    }

    LTLuaEventHandler(int node_ref, int func_ref, int filter,
            LTfloat left, LTfloat bottom, LTfloat right, LTfloat top)
        : LTEventHandler(filter, left, bottom, right, top)
    {
        LTLuaEventHandler::node_ref = node_ref;
        LTLuaEventHandler::func_ref = func_ref;
    }

    virtual bool consume(LTSceneNode *node, LTEvent *event) {
        get_weak_ref(g_L, node_ref);
        ltLuaGetRef(g_L, -1, func_ref);
        assert(lua_isfunction(g_L, -1));
        new (lt_alloc_LTEvent(g_L)) LTEvent(event); // push copy of event
        lua_pushvalue(g_L, -3); // push scene node
        lua_call(g_L, 2, 1);
        bool res;
        if (lua_isnil(g_L, -1)) {
            res = true; // consume by default.
        } else {
            res = lua_toboolean(g_L, -1);
        }
        lua_pop(g_L, 1); // pop res, node
        return res;
    }
};

static int add_event_handler(lua_State *L, int filter) {
    int nargs = ltLuaCheckNArgs(L, 2);
    LTSceneNode *node = lt_expect_LTSceneNode(L, 1);
    if (!lua_isfunction(L, 2)) {
        return luaL_error(L, "argument not a function");
    }
    int fref = ltLuaAddRef(L, 1, 2); // Add reference from node to handler func.
    int nref = make_weak_ref(L, 1);
    LTLuaEventHandler *handler;
    if (nargs == 2) {
        handler = new LTLuaEventHandler(nref, fref, filter);
    } else if (nargs == 3 && lt_is_LTMesh(L, 1)) {
        LTMesh *mesh = (LTMesh*)node;
        LTfloat border = luaL_checknumber(L, 3);
        LTfloat l, r, b, t;
        mesh->ensure_bb_uptodate();
        l = mesh->left - border;
        r = mesh->right + border;
        b = mesh->bottom - border;
        t = mesh->top + border;
        handler = new LTLuaEventHandler(nref, fref, filter, l, b, r, t);
    } else {
        LTfloat l = luaL_checknumber(L, 3);
        LTfloat b = -1000000.0f;
        LTfloat r = 1000000.0f;
        LTfloat t = 1000000.0f;
        if (nargs > 3) {
            b = luaL_checknumber(L, 4);
        }
        if (nargs > 4) {
            r = luaL_checknumber(L, 5);
        }
        if (nargs > 5) {
            t = luaL_checknumber(L, 6);
        }
        handler = new LTLuaEventHandler(nref, fref, filter, l, b, r, t);
    }
    node->add_event_handler(handler);
    lua_pushvalue(L, 1);
    return 1;
}

static int lt_AddEventHandler(lua_State *L) {
    return add_event_handler(L, 0);
}

static int lt_AddMouseHandler(lua_State *L) {
    return add_event_handler(L, LT_EVENT_MOUSE);
}

static int lt_AddMouseDownHandler(lua_State *L) {
    return add_event_handler(L, LT_EVENT_MOUSE_DOWN);
}

static int lt_AddMouseUpHandler(lua_State *L) {
    return add_event_handler(L, LT_EVENT_MOUSE_UP);
}

static int lt_AddMouseMoveHandler(lua_State *L) {
    return add_event_handler(L, LT_EVENT_MOUSE_MOVE);
}

static int lt_AddTouchHandler(lua_State *L) {
    return add_event_handler(L, LT_EVENT_TOUCH);
}

static int lt_AddTouchDownHandler(lua_State *L) {
    return add_event_handler(L, LT_EVENT_TOUCH_DOWN);
}

static int lt_AddTouchUpHandler(lua_State *L) {
    return add_event_handler(L, LT_EVENT_TOUCH_UP);
}

static int lt_AddTouchMoveHandler(lua_State *L) {
    return add_event_handler(L, LT_EVENT_TOUCH_MOVE);
}

static int lt_AddPointerHandler(lua_State *L) {
    return add_event_handler(L, LT_EVENT_POINTER);
}

static int lt_AddPointerDownHandler(lua_State *L) {
    return add_event_handler(L, LT_EVENT_POINTER_DOWN);
}

static int lt_AddPointerUpHandler(lua_State *L) {
    return add_event_handler(L, LT_EVENT_POINTER_UP);
}

static int lt_AddPointerMoveHandler(lua_State *L) {
    return add_event_handler(L, LT_EVENT_POINTER_MOVE);
}

static int lt_AddKeyHandler(lua_State *L) {
    return add_event_handler(L, LT_EVENT_KEY);
}

static int lt_AddKeyDownHandler(lua_State *L) {
    return add_event_handler(L, LT_EVENT_KEY_DOWN);
}

static int lt_AddKeyUpHandler(lua_State *L) {
    return add_event_handler(L, LT_EVENT_KEY_UP);
}

static int lt_PropagateEvent(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTSceneNode *node = lt_expect_LTSceneNode(L, 1);
    LTEvent *event = lt_expect_LTEvent(L, 2);
    ltPropagateEvent(node, event);
    return 0;
}

static int lt_MakeSceneNodeExclusive(lua_State *L) {
    int nargs = ltLuaCheckNArgs(L, 1);
    LTSceneNode *node = lt_expect_LTSceneNode(L, 1);
    bool exclusive = true;
    if (nargs > 1) {
        exclusive = lua_toboolean(L, 2);
    }
    if (exclusive && !node->active) {
        luaL_error(L, "scene node not active");
    }
    if (exclusive) {
        lt_exclusive_receiver = node;
    } else if (lt_exclusive_receiver == node) {
        lt_exclusive_receiver = NULL;
    }
    return 0;
}

/************************* Images **************************/

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

static void add_packer_images_to_lua_table(lua_State *L, int w, int h, LTImagePacker *packer, LTAtlas *atlas) {
    const char *name;
    if (packer->occupant != NULL) {
        name = packer->occupant->name;
        if (!packer->occupant->is_glyph) {
            new (lt_alloc_LTImage(L)) LTImage(atlas, w, h, packer);
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
            new (lt_alloc_LTImage(L)) LTImage(atlas, w, h, packer);
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

static LTTextureFilter decode_texture_filter_arg(lua_State *L, int arg) {
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
    // Load images named in 1st argument (an array) and return a table
    // indexed by image name.
    // The second and third arguments are the minimize and magnify
    // texture filters to use.
    // If an entry in the array is a table, then process it as a font.
    int num_args = ltLuaCheckNArgs(L, 1);
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

/************************* Models **************************/

static int lt_LoadModels(lua_State *L) {
    // Load wavefront models in 1st argument (an array) and return a table
    // of meshes indexed by model name.
    ltLuaCheckNArgs(L, 1);
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
        const char *path = model_path(name); 
        LTMesh *mesh = (LTMesh*)lt_alloc_LTMesh(L);
        if (!ltReadWavefrontMesh(path, mesh)) {
            return luaL_error(L, "Unable to read model at path %s", path);
        }
        mesh->setup();
        delete[] path;
        lua_setfield(L, -2, name);
        i++;
    }
    return 1;
}

/************************* Audio **************************/

static int lt_LoadSamples(lua_State *L) {
    // Load sounds in 1st argument (an array) and return a table
    // indexed by sound name.
    ltLuaCheckNArgs(L, 1);
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
        LTAudioSample *sample = ltReadAudioSample(L, path, name);
        delete[] path;
        if (sample != NULL) {
            // If sample is NULL ltReadAudioSample would have already logged an error.
            lua_setfield(L, -2, name);
        }
        i++;
    }
    return 1;
}

static int lt_PlaySampleOnce(lua_State *L) {
    int num_args = ltLuaCheckNArgs(L, 1);
    LTfloat pitch = 1.0f;
    LTfloat gain = 1.0f;
    if (num_args > 1) {
        pitch = luaL_checknumber(L, 2);
    }
    if (num_args > 2) {
        gain = luaL_checknumber(L, 3);
    }
    LTAudioSample *sample = lt_expect_LTAudioSample(L, 1);
    sample->play(pitch, gain);
    return 0;
}

static int lt_PlayTrack(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTTrack *track = lt_expect_LTTrack(L, 1);
    track->play();
    return 0;
}

static int lt_PauseTrack(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTTrack *track = lt_expect_LTTrack(L, 1);
    track->pause();
    return 0;
}

static int lt_StopTrack(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTTrack *track = lt_expect_LTTrack(L, 1);
    track->stop();
    return 0;
}

static int lt_QueueSampleInTrack(lua_State *L) {
    int num_args = ltLuaCheckNArgs(L, 2);
    LTTrack *track = lt_expect_LTTrack(L, 1);
    LTAudioSample *sample = lt_expect_LTAudioSample(L, 2);
    int n = 1;
    if (num_args > 2) {
        n = luaL_checkinteger(L, 3);
    }
    for (int i = 0; i < n; i++) {
        int ref = ltLuaAddRef(L, 1, 2); // Add ref from track to sample.
        track->queueSample(sample, ref);
    }
    return 0;
}

static int lt_SetTrackLoop(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTTrack *track = lt_expect_LTTrack(L, 1);
    bool loop = lua_toboolean(L, 2);
    track->setLoop(loop);
    return 0;
}

static int lt_TrackQueueSize(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTTrack *track = lt_expect_LTTrack(L, 1);
    lua_pushinteger(L, track->numSamples());
    return 1;
}

static int lt_TrackNumPlayed(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTTrack *track = lt_expect_LTTrack(L, 1);
    lua_pushinteger(L, track->numProcessedSamples());
    return 1;
}

static int lt_TrackNumPending(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTTrack *track = lt_expect_LTTrack(L, 1);
    lua_pushinteger(L, track->numPendingSamples());
    return 1;
}

static int lt_TrackDequeuePlayed(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTTrack *track = lt_expect_LTTrack(L, 1);
    int n = (int)luaL_checkinteger(L, 2);
    int processed = track->numProcessedSamples();
    if (n > processed) {
        n = processed;
    }
    track->dequeueSamples(L, 1, n);
    return 0;
}

static int lt_SampleNumDataPoints(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTAudioSample *sample = lt_expect_LTAudioSample(L, 1);
    lua_pushinteger(L, sample->numDataPoints());
    return 1;
}

static int lt_SampleFrequency(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTAudioSample *sample = lt_expect_LTAudioSample(L, 1);
    lua_pushinteger(L, sample->dataPointsPerSec());
    return 1;
}

static int lt_SampleLength(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTAudioSample *sample = lt_expect_LTAudioSample(L, 1);
    lua_pushnumber(L, sample->length());
    return 1;
}

/************************* Box2D **************************/

/*
static int lt_FixtureContainsPoint(lua_State *L) {
    ltLuaCheckNArgs(L, 3); 
    LTFixture *fixture = lt_expect_LTFixture(L, 1);
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
    ltLuaCheckNArgs(L, 1); 
    LTFixture *fixture = lt_expect_LTFixture(L, 1);
    fixture->destroy();
    LTBody *body = fixture->body;
    if (body != NULL) {
        lt_pushfield_LTFixture
        lt_unset_LTFixture_body(L, 1); // Remove reference from fixture to body.
        ltLuaPushRef(L, 1, body);
        ltLuaDelRef(L, -1, 1); // Remove reference from body to fixture.
        ltLuaDelRef(L, 1, -1); 
        lua_pop(L, 2);
    }
    return 0;
}

static int lt_FixtureIsDestroyed(lua_State *L) {
    ltLuaCheckNArgs(L, 1); 
    LTFixture *fixture = (LTFixture*)get_object(L, 1, LT_TYPE_FIXTURE);
    lua_pushboolean(L, fixture->fixture == NULL);
    return 1;
}

static int lt_DoWorldStep(lua_State *L) {
    int num_args = ltLuaCheckNArgs(L, 2); 
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
    ltLuaCheckNArgs(L, 3);
    LTWorld *world = (LTWorld*)get_object(L, 1, LT_TYPE_WORLD);
    LTfloat x = (LTfloat)luaL_checknumber(L, 2);
    LTfloat y = (LTfloat)luaL_checknumber(L, 3);
    world->world->SetGravity(b2Vec2(x, y));
    return 0;
}

static int lt_SetWorldAutoClearForces(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
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
    ltLuaCheckNArgs(L, 5);
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
    ltLuaCheckNArgs(L, 1);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    body->destroy();
    b2Body *b = body->body;
    if (b != NULL) {
        // Remove references between body and its fixtures.
        b2Fixture *f = b->GetFixtureList();
        while (f != NULL) {
            LTFixture *ud = (LTFixture*)f->GetUserData();
            get_weak_ref(L, ud->lua_wrap);
            ltLuaDelRef(L, 1, -1); // Remove reference from body to fixture.
            ltLuaDelRef(L, -1, 1); // Remove reference from fixture to body.
            lua_pop(L, 1);
            f = f->GetNext();
        }
    }
    LTWorld *world = body->world;
    get_weak_ref(L, world->lua_wrap);
    ltLuaDelRef(L, -1, 1); // Remove reference from world to body.
    ltLuaDelRef(L, 1, -1); // Remove reference from body to world.
    lua_pop(L, 1);
    return 0;
}

static int lt_BodyIsDestroyed(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    lua_pushboolean(L, body->body == NULL);
    return 1;
}

static int lt_ApplyForceToBody(lua_State *L) {
    int num_args = ltLuaCheckNArgs(L, 3);
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
    ltLuaCheckNArgs(L, 2);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        body->body->ApplyTorque(luaL_checknumber(L, 2));
    }
    return 0;
}

static int lt_ApplyImpulseToBody(lua_State *L) {
    int num_args = ltLuaCheckNArgs(L, 3);
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
    ltLuaCheckNArgs(L, 2);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        body->body->ApplyAngularImpulse(luaL_checknumber(L, 2));
    }
    return 0;
}

//
// * m_forces and m_torque are private members.
//static int lt_ClearBodyForces(lua_State *L) {
//    ltLuaCheckNArgs(L, 1);
//    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
//    if (body->body != NULL) {
//        body->body->m_force.SetZero();
//        body->body->m_torque = 0.0f;
//    }
//    return 0;
//}
//

static int lt_GetBodyAngle(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        lua_pushnumber(L, body->body->GetAngle() * LT_DEGREES_PER_RADIAN);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lt_SetBodyAngle(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        body->body->SetTransform(body->body->GetPosition(), luaL_checknumber(L, 2) * LT_RADIANS_PER_DEGREE);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lt_GetBodyPosition(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
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
    ltLuaCheckNArgs(L, 1);
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
    ltLuaCheckNArgs(L, 3);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        b2Vec2 v = b2Vec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
        body->body->SetLinearVelocity(v);
    }
    return 0;
}

static int lt_SetBodyPosition(lua_State *L) {
    ltLuaCheckNArgs(L, 3);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    LTfloat x = luaL_checknumber(L, 2);
    LTfloat y = luaL_checknumber(L, 3);
    if (body->body != NULL) {
        body->body->SetTransform(b2Vec2(x, y), body->body->GetAngle());
    }
    return 0;
}

static int lt_SetBodyAngularVelocity(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        body->body->SetAngularVelocity(luaL_checknumber(L, 2) * LT_RADIANS_PER_DEGREE);
    }
    return 0;
}

static int lt_SetBodyGravityScale(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        body->body->SetGravityScale(luaL_checknumber(L, 2));
    }
    return 0;
}

static int lt_AddRectToBody(lua_State *L) {
    int num_args = ltLuaCheckNArgs(L, 5);
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
        ltLuaAddRef(L, 1, -1); // Add reference from body to new fixture.
        ltLuaAddRef(L, -1, 1); // Add reference from fixture to body.
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lt_AddTriangleToBody(lua_State *L) {
    int num_args = ltLuaCheckNArgs(L, 7);
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
        ltLuaAddRef(L, 1, -1); // Add reference from body to new fixture.
        ltLuaAddRef(L, -1, 1); // Add reference from fixture to body.
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
    ltLuaCheckNArgs(L, 3);
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
        ltLuaAddRef(L, 1, -1); // Add reference from body to new fixture.
        set_ref_field(L, -1, "body", 1); // Add reference from fixture to body.
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lt_AddCircleToBody(lua_State *L) {
    ltLuaCheckNArgs(L, 5);
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
        ltLuaAddRef(L, 1, -1); // Add reference from body to new fixture.
        set_ref_field(L, -1, "body", 1); // Add reference from fixture to body.
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lt_GetFixtureBody(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTFixture *fixture = (LTFixture*)get_object(L, 1, LT_TYPE_FIXTURE);
    if (fixture->fixture != NULL && fixture->body != NULL) {
        push_wrap(L, fixture->body);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lt_GetBodyFixtures(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
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
    ltLuaCheckNArgs(L, 1);
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
    ltLuaCheckNArgs(L, 1);
    LTWorld *world = (LTWorld*)get_object(L, 1, LT_TYPE_WORLD);
    b2BodyDef def;
    def.type = b2_staticBody;
    LTBody *body = new LTBody(world, &def);
    push_wrap(L, body);
    ltLuaAddRef(L, 1, -1); // Add reference from world to body.
    ltLuaAddRef(L, -1, 1); // Add reference from body to world.
    return 1;
}

static int lt_AddDynamicBodyToWorld(lua_State *L) {
    int num_args = ltLuaCheckNArgs(L, 3);
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
    ltLuaAddRef(L, 1, -1); // Add reference from world to body.
    ltLuaAddRef(L, -1, 1); // Add reference from body to world.
    return 1;
}

static int lt_AddBodyToWorld(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
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
    ltLuaAddRef(L, 1, -1); // Add reference from world to body.
    ltLuaAddRef(L, -1, 1); // Add reference from body to world.
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
    ltLuaCheckNArgs(L, 2);
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
    ltLuaAddRef(L, 1, -1); // Add reference from world to joint.
    ltLuaAddRef(L, -1, 1); // Add reference from joint to world.
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
    int nargs = ltLuaCheckNArgs(L, 1);
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
    ltLuaCheckNArgs(L, 5);
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
    int nargs = ltLuaCheckNArgs(L, 0);
    LTfloat scaling = 1.0f;
    if (nargs > 0) {
        scaling = luaL_checknumber(L, 1);
    }
    LTWorld *world = new LTWorld(b2Vec2(0.0f, -10.0f), true, scaling);
    push_wrap(L, world);
    return 1;
}

static int lt_BodyTracker(lua_State *L) {
    int nargs = ltLuaCheckNArgs(L, 2);
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
*/

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
    ltLuaCheckNArgs(L, 2);
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
    ltLuaCheckNArgs(L, 1);
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
    ltLuaCheckNArgs(L, 1);
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

/********************* Actions ****************************/

struct LTLuaAction : LTAction {
    int node_ref;
    int func_ref;
    LTdouble t_accum;

    LTLuaAction(LTSceneNode *node, int node_ref, int func_ref) : LTAction(node) {
        LTLuaAction::node_ref = node_ref;
        LTLuaAction::func_ref = func_ref;
        t_accum = 0.0;
    }

    virtual ~LTLuaAction() {
    }

    virtual void on_cancel() {
        get_weak_ref(g_L, node_ref);
        assert(lt_is_LTSceneNode(g_L, -1));
        ltLuaDelRef(g_L, -1, func_ref);
        del_weak_ref(g_L, node_ref);
        lua_pop(g_L, 1);
    }

    virtual bool doAction(LTfloat fdt) {
        bool res = false;
        LTdouble dt = (LTdouble)fdt;
        t_accum += dt;
        get_weak_ref(g_L, node_ref);
        while (t_accum > 0.0) {
            ltLuaGetRef(g_L, -1, func_ref);
            assert(lua_isfunction(g_L, -1));
            lua_pushnumber(g_L, dt);
            lua_pushvalue(g_L, -3); // push scene node
            lua_call(g_L, 2, 1);
            if (lua_type(g_L, -1) == LUA_TNUMBER) {
                t_accum -= lua_tonumber(g_L, -1);
            } else {
                res = lua_toboolean(g_L, -1);
                t_accum = 0.0;
            }
            lua_pop(g_L, 1); // pop res
        }
        lua_pop(g_L, 1); // pop node
        return res;
    }
};

static int lt_AddAction(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTSceneNode *node = lt_expect_LTSceneNode(L, 1);
    if (!lua_isfunction(L, 2)) {
        return luaL_error(L, "argument not a function");
    }
    int fref = ltLuaAddRef(L, 1, 2); // Add reference from node to action func.
    int nref = make_weak_ref(L, 1);
    LTAction *action = new LTLuaAction(node, nref, fref);
    node->add_action(action);
    lua_pushvalue(L, 1);
    return 1;
}

static int lt_ExecuteActions(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    ltExecuteActions((LTfloat)luaL_checknumber(L, 1));
    return 0;
}

/************************* Tweens **************************/

struct LTLuaTweenOnDone : LTTweenOnDone {
    LTSceneNode *node;
    int node_ref;
    int func_ref;
    bool executed;
    bool cancelled;

    LTLuaTweenOnDone(LTSceneNode *node, int nref, int fref) {
        LTLuaTweenOnDone::node = node;
        node_ref = nref;
        func_ref = fref;
        executed = false;
        cancelled = false;
    }
    virtual void on_cancel() {
        assert(!cancelled);
        get_weak_ref(g_L, node_ref);
        assert(lua_touserdata(g_L, -1) == node);
        ltLuaDelRef(g_L, -1, func_ref);
        del_weak_ref(g_L, node_ref);
        lua_pop(g_L, 1);
        cancelled = true;
    }
    virtual void done(LTTweenAction *action) {
        assert(action->node == node);
        assert(!executed);
        assert(!cancelled);
        get_weak_ref(g_L, node_ref);
        assert(node == lua_touserdata(g_L, -1));
        ltLuaGetRef(g_L, -1, func_ref);
        assert(lua_isfunction(g_L, -1));
        lua_call(g_L, 0, 0);
        lua_pop(g_L, 1); // pop node
        executed = true;
    }
};

static int lt_AddTween(lua_State *L) {
    ltLuaCheckNArgs(L, 7); // node, field, target_val, time, delay, easing, action
    ltLuaFindFieldOwner(L, 1, 2);
    if (lua_isnil(L, -1)) {
        return luaL_error(L, "Field %s does not exist, or object not a scene node", lua_tostring(L, 2));
    }
    LTSceneNode *node = (LTSceneNode*)lua_touserdata(L, -1);
    LTFloatGetter getter;
    LTFloatSetter setter;
    ltLuaGetFloatGetterAndSetter(L, -1, 2, &getter, &setter);
    if (getter == NULL) {
        return luaL_error(L, "Field %s is not a float", lua_tostring(L, 2));
    }
    if (setter == NULL) {
        return luaL_error(L, "Field %s is read-only", lua_tostring(L, 2));
    }
    LTfloat target_val = luaL_checknumber(L, 3);
    LTfloat time = luaL_checknumber(L, 4);
    LTfloat delay = luaL_checknumber(L, 5);
    if (!(lua_isnil(L, 6) || lua_isstring(L, 6))) {
        return luaL_error(L, "Ease func argument (6) not nil or a string");
    }
    const char *ease_func_str = lua_tostring(L, 6);
    LTEaseFunc ease_func;
    if (ease_func_str == NULL) {
        ease_func = ltEase_linear;
    } else if (strcmp(ease_func_str, "in") == 0) {
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
    LTLuaTweenOnDone *on_done = NULL;
    if (lua_isfunction(L, 7)) {
        int fref = ltLuaAddRef(L, -1, 7); // Add reference from node to action func.
        int nref = make_weak_ref(L, -1);
        on_done = new LTLuaTweenOnDone(node, nref, fref);
    } else if (!lua_isnil(L, 7)) {
        return luaL_error(L, "Argument 7 not a function or nil");
    }
    LTTweenAction *action = new LTTweenAction(node, getter, setter, target_val, time, delay, ease_func, on_done);
    node->add_action(action);
    return 0;
}

/*
static int lt_MakeNativeTween(lua_State *L) {
    LTObject *obj = lt_expect_LTObject(L, 1);
    LTFloatGetter getter;
    LTFloatSetter setter;
    ltLuaGetFloatGetterAndSetter(L, 1, 2, &getter, &setter);
    if (getter == NULL || setter == NULL) {
        lua_pushnil(L);
        return 1;
    }
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
    ltInitTween(tween, obj, getter, setter, value, time, delay, ease_func);
    return 1;
}

static int lt_AdvanceNativeTween(lua_State *L) {
    LTTween *tween = (LTTween*)lua_touserdata(L, 1);
    LTfloat dt = luaL_checknumber(L, 2);
    lua_pushboolean(L, ltAdvanceTween(tween, dt) ? 1 : 0);
    return 1;
}
*/

/************************* Random numbers **************************/

//static int lt_Random(lua_State *L) {
//    ltLuaCheckNArgs(L, 1);
//    int seed = luaL_checkinteger(L, 1);
//    LTRandomGenerator *r = new LTRandomGenerator(seed);
//    push_wrap(L, r);
//    return 1;
//}
//
//static int lt_NextRandomInt(lua_State *L) {
//    int nargs = ltLuaCheckNArgs(L, 2);
//    LTRandomGenerator *r = (LTRandomGenerator*)get_object(L, 1, LT_TYPE_RANDOMGENERATOR);
//    if (nargs > 2) {
//        int min = luaL_checkinteger(L, 2);
//        int max = luaL_checkinteger(L, 3);
//        lua_pushinteger(L, r->nextInt(max - min + 1) + min);
//    } else {
//        int max = luaL_checkinteger(L, 2);
//        lua_pushinteger(L, r->nextInt(max) + 1);
//    }
//    return 1;
//}
//
//static int lt_NextRandomNumber(lua_State *L) {
//    int nargs = ltLuaCheckNArgs(L, 1);
//    LTRandomGenerator *r = (LTRandomGenerator*)get_object(L, 1, LT_TYPE_RANDOMGENERATOR);
//    if (nargs == 1) {
//        lua_pushnumber(L, r->nextDouble());
//    }
//    if (nargs == 2) {
//        LTdouble scale = luaL_checknumber(L, 2);
//        lua_pushnumber(L, r->nextDouble() * scale);
//    } else {
//        LTdouble min = luaL_checknumber(L, 2);
//        LTdouble max = luaL_checknumber(L, 3);
//        lua_pushnumber(L, min + (max - min) * r->nextDouble());
//    }
//    return 1;
//}
//
//static int lt_NextRandomBool(lua_State *L) {
//    ltLuaCheckNArgs(L, 1);
//    LTRandomGenerator *r = (LTRandomGenerator*)get_object(L, 1, LT_TYPE_RANDOMGENERATOR);
//    lua_pushboolean(L, r->nextBool());
//    return 1;
//}

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
    int nargs = ltLuaCheckNArgs(L, 1);
    const char *module = lua_tostring(L, 1);
    if (module == NULL) {
        return luaL_error(L, "The import function requires a string argument.");
    }
    const char *path;
    path = resource_path(module, ".lua");
    LTResource *rsc = ltOpenResource(path);
    if (rsc != NULL) {
        int r = loadfile(L, rsc);
        delete[] path;
        ltCloseResource(rsc);
        if (r != 0) {
            const char *msg = lua_tostring(L, -1);
            lua_pop(L, 1);
            return luaL_error(L, "%s", msg);
        }
        for (int i = 2; i <= nargs; i++) {
            lua_pushvalue(L, i);
        }
        lua_call(L, nargs - 1, LUA_MULTRET);
        return lua_gettop(L) - top;
    } else {
        return luaL_error(L, "File %s does no exist", path);
    }
}

/************************ Configuration *****************************/

static int lt_SetAppShortName(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    const char *short_name = lua_tostring(L, 1);
    if (short_name != NULL) {
        if (lt_app_short_name == NULL) {
            delete[] lt_app_short_name;
        }
        char *tmp = new char[strlen(short_name) + 1];
        strcpy(tmp, short_name);
        lt_app_short_name = tmp;
    } else {
        luaL_error(L, "Expecting one string argument");
    }
    return 0;
}

/************************ Logging *****************************/

static int log(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
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

    {"DrawSceneNode",                   lt_DrawSceneNode},
    {"InsertLayerFront",                lt_InsertLayerFront},
    {"InsertLayerBack",                 lt_InsertLayerBack},
    {"InsertLayerAbove",                lt_InsertLayerAbove},
    {"InsertLayerBelow",                lt_InsertLayerBelow},
    {"RemoveFromLayer",                 lt_RemoveFromLayer},
    {"LayerSize",                       lt_LayerSize},

    {"AddEventHandler",                 lt_AddEventHandler},
    {"AddMouseHandler",                 lt_AddMouseHandler},
    {"AddMouseDownHandler",             lt_AddMouseDownHandler},
    {"AddMouseUpHandler",               lt_AddMouseUpHandler},
    {"AddMouseMoveHandler",             lt_AddMouseMoveHandler},
    {"AddTouchHandler",                 lt_AddTouchHandler},
    {"AddTouchDownHandler",             lt_AddTouchDownHandler},
    {"AddTouchUpHandler",               lt_AddTouchUpHandler},
    {"AddTouchMoveHandler",             lt_AddTouchMoveHandler},
    {"AddPointerHandler",               lt_AddPointerHandler},
    {"AddPointerDownHandler",           lt_AddPointerDownHandler},
    {"AddPointerUpHandler",             lt_AddPointerUpHandler},
    {"AddPointerMoveHandler",           lt_AddPointerMoveHandler},
    {"AddKeyHandler",                   lt_AddKeyHandler},
    {"AddKeyUpHandler",                 lt_AddKeyUpHandler},
    {"AddKeyDownHandler",               lt_AddKeyDownHandler},
    {"PropagateEvent",                  lt_PropagateEvent},
    {"MakeSceneNodeExclusive",          lt_MakeSceneNodeExclusive},

    {"LoadImages",                      lt_LoadImages},

    {"Vector",                          lt_Vector},
    {"GenerateVectorColumn",            lt_GenerateVectorColumn},
    {"FillVectorColumnsWithImageQuads", lt_FillVectorColumnsWithImageQuads},
    //{"DrawQuads",                       lt_DrawQuads},

    {"LoadModels",                      lt_LoadModels},

    //{"ParticleSystemFixtureFilter",     lt_ParticleSystemFixtureFilter},

    {"AddTween",                        lt_AddTween},
    //{"MakeNativeTween",                 lt_MakeNativeTween},
    //{"AdvanceNativeTween",              lt_AdvanceNativeTween},
    //{"TweenSet",                        lt_TweenSet},
    //{"AdvanceTweens",                   lt_AdvanceTweens},
    //{"ClearTweens",                     lt_ClearTweens},

    {"AddAction",                       lt_AddAction},
    {"ExecuteActions",                  lt_ExecuteActions},

    {"LoadSamples",                     lt_LoadSamples},
    {"PlaySampleOnce",                  lt_PlaySampleOnce},
    {"PlayTrack",                       lt_PlayTrack},
    {"PauseTrack",                      lt_PauseTrack},
    {"StopTrack",                       lt_StopTrack},
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

//    {"World",                           lt_World},
//    {"FixtureContainsPoint",            lt_FixtureContainsPoint},
//    {"DestroyFixture",                  lt_DestroyFixture},
//    {"FixtureIsDestroyed",              lt_FixtureIsDestroyed},
//    {"DoWorldStep",                     lt_DoWorldStep},
//    {"SetWorldGravity",                 lt_SetWorldGravity},
//    {"SetWorldAutoClearForces",         lt_SetWorldAutoClearForces},
//    {"WorldQueryBox",                   lt_WorldQueryBox},
//    {"DestroyBody",                     lt_DestroyBody},
//    {"BodyIsDestroyed",                 lt_BodyIsDestroyed},
//    {"ApplyForceToBody",                lt_ApplyForceToBody},
//    {"ApplyTorqueToBody",               lt_ApplyTorqueToBody},
//    {"ApplyImpulseToBody",              lt_ApplyImpulseToBody},
//    {"ApplyAngularImpulseToBody",       lt_ApplyAngularImpulseToBody},
//    //{"ClearBodyForces",                 lt_ClearBodyForces},
//    {"GetBodyAngle",                    lt_GetBodyAngle},
//    {"SetBodyAngle",                    lt_SetBodyAngle},
//    {"GetBodyPosition" ,                lt_GetBodyPosition},
//    {"SetBodyPosition" ,                lt_SetBodyPosition},
//    {"GetBodyVelocity" ,                lt_GetBodyVelocity},
//    {"SetBodyVelocity" ,                lt_SetBodyVelocity},
//    {"SetBodyAngularVelocity",          lt_SetBodyAngularVelocity},
//    {"SetBodyGravityScale",             lt_SetBodyGravityScale},
//    {"AddRectToBody",                   lt_AddRectToBody},
//    {"AddTriangleToBody",               lt_AddTriangleToBody},
//    {"AddPolygonToBody",                lt_AddPolygonToBody},
//    {"AddCircleToBody",                 lt_AddCircleToBody},
//    {"GetFixtureBody",                  lt_GetFixtureBody},
//    {"GetBodyFixtures",                 lt_GetBodyFixtures},
//    {"FixtureBoundingBox",              lt_FixtureBoundingBox},
//    {"AddStaticBodyToWorld",            lt_AddStaticBodyToWorld},
//    {"AddDynamicBodyToWorld",           lt_AddDynamicBodyToWorld},
//    {"AddBodyToWorld",                  lt_AddBodyToWorld},
//    {"AddJointToWorld",                 lt_AddJointToWorld},
//    {"BodyOrFixtureTouching",           lt_BodyOrFixtureTouching},
//    {"BodyTracker",                     lt_BodyTracker},
//    {"WorldRayCast",                    lt_WorldRayCast},

    {"GameCenterAvailable",             lt_GameCenterAvailable},
    {"SubmitScore",                     lt_SubmitScore},
    {"ShowLeaderboard",                 lt_ShowLeaderboard},

    {"OpenURL",                         lt_OpenURL},

//    {"Random",                          lt_Random},
//    {"NextRandomInt",                   lt_NextRandomInt},
//    {"NextRandomNumber",                lt_NextRandomNumber},
//    {"NextRandomBool",                  lt_NextRandomBool},

    {"SetAppShortName",                 lt_SetAppShortName},

    {NULL, NULL}
};

/************************************************************/

static bool push_lt_func(lua_State *L, const char *func) {
    lua_getglobal(L, "lt");
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, func);
        lua_remove(L, -2); // Remove lt table.
        if (lua_isfunction(L, -1)) {
            return true;
        } else {
            lua_pop(L, 1); // Pop the field since we won't be calling it.
            return false;
        }
    } else {
        lua_pop(L, 1);
        return false;
    }
}

static void call_lt_func(lua_State *L, const char *func) {
    if (push_lt_func(L, func)) {
        docall(L, 0, 0);
    }
}

static void run_lua_file(lua_State *L, const char *file) {
    if (!g_suspended) {
        const char *f = resource_path(file, ".lua");
        LTResource *r = ltOpenResource(f);
        if (r != NULL) {
            check_status(L, loadfile(L, r));
            docall(L, 0, 0);
            ltCloseResource(r);
        } else {
            ltLog("File %s does not exist", f);
        }
        delete[] f;
    }
}

static void setup_wref_ref(lua_State *L) {
    lua_getglobal(L, "lt");
    lua_getfield(L, -1, "wrefs");
    g_wrefs_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pop(L, 1); // pop lt.
}

static void set_viewport_globals(lua_State *L) {
    if (L != NULL) {
        lua_getglobal(L, "lt");
        lua_pushnumber(L, ltGetViewPortLeftEdge());
        lua_setfield(L, -2, "left");
        lua_pushnumber(L, ltGetViewPortBottomEdge());
        lua_setfield(L, -2, "bottom");
        lua_pushnumber(L, ltGetViewPortRightEdge());
        lua_setfield(L, -2, "right");
        lua_pushnumber(L, ltGetViewPortTopEdge());
        lua_setfield(L, -2, "top");
        lua_pushnumber(L, ltGetViewPortRightEdge() - ltGetViewPortLeftEdge());
        lua_setfield(L, -2, "width");
        lua_pushnumber(L, ltGetViewPortTopEdge() - ltGetViewPortBottomEdge());
        lua_setfield(L, -2, "height");
        lua_pop(L, 1); // pop lt
    }
}

static void set_globals(lua_State *L) {
    if (L != NULL) {
        lua_getglobal(L, "lt");
        #ifdef LTIOS
            lua_pushboolean(L, 1);
            lua_setfield(L, -2, "ios");
            lua_pushboolean(L, ltIsIPad() ? 1 : 0);
            lua_setfield(L, -2, "ipad");
            lua_pushboolean(L, ltIOSSupportsES2());
            lua_setfield(L, -2, "shaders");
            #ifdef LTADS
                if (LTADS == LT_AD_TOP) {
                    lua_pushstring(L, "top");
                } else {
                    lua_pushstring(L, "bottom");
                }
                lua_setfield(L, -2, "ads");
            #endif
        #endif
        #ifdef LTOSX
            lua_pushboolean(L, 1);
            lua_setfield(L, -2, "osx");
            lua_pushboolean(L, 1);
            lua_setfield(L, -2, "desktop");
            lua_pushboolean(L, 1);
            lua_setfield(L, -2, "shaders");
        #endif
        lua_pop(L, 1); // pop lt
    }
}

void ltLuaSetup() {
    #ifndef LTANDROID
    ltAudioInit();
    #endif
    g_L = luaL_newstate();
    if (g_L == NULL) {
        ltLog("Cannot create lua state: not enough memory.");
        ltAbort();
    }
    ltLuaInitFFI(g_L);
    luaL_openlibs(g_L);
    lua_pushcfunction(g_L, import);
    lua_setglobal(g_L, "import");
    lua_pushcfunction(g_L, log);
    lua_setglobal(g_L, "log");
    luaL_register(g_L, "lt", ltlib);
    lua_pop(g_L, 1); // pop lt
    run_lua_file(g_L, "lt");
    setup_wref_ref(g_L);
    set_globals(g_L);
    run_lua_file(g_L, "config");
    ltRestoreState();
}

void ltLuaSetResourcePrefix(const char *prefix) {
    resource_prefix = prefix;
}

void ltLuaTeardown() {
    if (g_L != NULL) {
        ltDeactivateAllScenes(g_L);
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
    if (g_L != NULL && !g_suspended && push_lt_func(g_L, "Advance")) {
        lua_pushnumber(g_L, secs);
        docall(g_L, 1, 0);
    }
    ltAudioGC();
}

void ltLuaRender() {
    if (g_L != NULL && !g_suspended) {
        if (!g_initialized) {
            ltInitGLState();
            ltAdjustViewportAspectRatio();
            set_viewport_globals(g_L);
            run_lua_file(g_L, "main");
            #ifdef LTGAMECENTER
            if (ltIOSGameCenterIsAvailable()) {
                ltLuaGameCenterBecameAvailable();
            }
            #endif
            g_initialized = true;
        }
        if (!g_suspended) {
            ltInitGraphics();
            call_lt_func(g_L, "Render");
            ltDrawAdBackground();
            ltDrawConnectingOverlay();
        }
    }
}

static void handle_event(LTEvent *e) {
    if (g_L != NULL && !g_suspended && push_lt_func(g_L, "HandleEvent")) {
        new (lt_alloc_LTEvent(g_L)) LTEvent(e);
        docall(g_L, 1, 0);
    }
}

void ltLuaMouseDown(int button, LTfloat x, LTfloat y) {
    LTEvent e;
    e.event = LT_EVENT_MOUSE_DOWN;
    e.x = ltGetViewPortX(x);
    e.y = ltGetViewPortY(y);
    e.orig_x = x;
    e.orig_y = y;
    e.button = button;
    handle_event(&e);
}

void ltLuaMouseUp(int button, LTfloat x, LTfloat y) {
    LTEvent e;
    e.event = LT_EVENT_MOUSE_UP;
    e.x = ltGetViewPortX(x);
    e.y = ltGetViewPortY(y);
    e.orig_x = x;
    e.orig_y = y;
    e.button = button;
    handle_event(&e);
}

void ltLuaMouseMove(LTfloat x, LTfloat y) {
    LTEvent e;
    e.event = LT_EVENT_MOUSE_MOVE;
    e.x = ltGetViewPortX(x);
    e.y = ltGetViewPortY(y);
    e.orig_x = x;
    e.orig_y = y;
    handle_event(&e);
}

void ltLuaTouchDown(int touch_id, LTfloat x, LTfloat y) {
    LTEvent e;
    e.event = LT_EVENT_TOUCH_DOWN;
    e.x = ltGetViewPortX(x);
    e.y = ltGetViewPortY(y);
    e.orig_x = x;
    e.orig_y = y;
    e.touch_id = touch_id;
    handle_event(&e);
}

void ltLuaTouchUp(int touch_id, LTfloat x, LTfloat y) {
    LTEvent e;
    e.event = LT_EVENT_TOUCH_UP;
    e.x = ltGetViewPortX(x);
    e.y = ltGetViewPortY(y);
    e.orig_x = x;
    e.orig_y = y;
    e.touch_id = touch_id;
    handle_event(&e);
}

void ltLuaTouchMove(int touch_id, LTfloat x, LTfloat y) {
    LTEvent e;
    e.event = LT_EVENT_TOUCH_MOVE;
    e.x = ltGetViewPortX(x);
    e.y = ltGetViewPortY(y);
    e.orig_x = x;
    e.orig_y = y;
    e.touch_id = touch_id;
    handle_event(&e);
}

void ltLuaKeyDown(LTKey key) {
    LTEvent e;
    e.event = LT_EVENT_KEY_DOWN;
    e.key = key;
    handle_event(&e);
}

void ltLuaKeyUp(LTKey key) {
    LTEvent e;
    e.event = LT_EVENT_KEY_UP;
    e.key = key;
    handle_event(&e);
}

/************************************************************/

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
        push_lt_func(g_L, "GameCenterBecameAvailable"))
    {
        docall(g_L, 0, 0);
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
    if (g_L != NULL) {
        lua_getglobal(g_L, "lt");
        if (unpickler != NULL) {
            unpickle_value(g_L, unpickler);
        } else {
            lua_pushnil(g_L);
        }
        if (lua_isnil(g_L, -1)) {
            lua_pop(g_L, 1);
            lua_newtable(g_L);
        }
        lua_setfield(g_L, -2, "state");
        lua_pop(g_L, 1);
    }
}

/************************************************************/

//void ltLuaPreContextChange() {
//    if (g_L == NULL) {
//        return;
//    }
//    lua_State *L = g_L;
//    // Traverse weak refs table, calling preContextChange method on 
//    // all scene nodes.
//    lua_rawgeti(L, LUA_REGISTRYINDEX, g_wrefs_ref);
//    lua_pushnil(L);
//    while (lua_next(L, -2) != 0) {
//        if (lua_istable(g_L, -1)) {
//            lua_rawgeti(L, LUA_REGISTRYINDEX, g_userdata_key_ref);
//            void *ud = lua_touserdata(L, -1);
//            if (ud != NULL) {
//                LTObject *obj = (LTObject*)ud;
//                if (obj->hasType(LT_TYPE_SCENENODE)) {
//                    LTSceneNode *node = (LTSceneNode*)obj;
//                    node->preContextChange();
//                }
//            }
//            lua_pop(L, 1); // pop userdata
//        }
//        lua_pop(L, 1); // pop value (key now on top of stack).
//    }
//    lua_pop(L, 1); // pop wrefs table.
//}
//
//void ltLuaPostContextChange() {
//    if (g_L == NULL) {
//        return;
//    }
//    lua_State *L = g_L;
//    // Traverse weak refs table, calling preContextChange method on 
//    // all scene nodes.
//    lua_rawgeti(L, LUA_REGISTRYINDEX, g_wrefs_ref);
//    lua_pushnil(L);
//    while (lua_next(L, -2) != 0) {
//        if (lua_istable(g_L, -1)) {
//            lua_rawgeti(L, LUA_REGISTRYINDEX, g_userdata_key_ref);
//            void *ud = lua_touserdata(L, -1);
//            if (ud != NULL) {
//                LTObject *obj = (LTObject*)ud;
//                if (obj->hasType(LT_TYPE_SCENENODE)) {
//                    LTSceneNode *node = (LTSceneNode*)obj;
//                    node->postContextChange();
//                }
//            }
//            lua_pop(L, 1); // pop userdata
//        }
//        lua_pop(L, 1); // pop value (key now on top of stack).
//    }
//    lua_pop(L, 1); // pop wrefs table.
//}
