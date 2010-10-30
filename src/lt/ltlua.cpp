/* Copyright (C) 2010 Ian MacLarty */

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "Box2D/Box2D.h"
#include "ltgraphics.h"
#include "ltharness.h"
#include "ltlua.h"

static lua_State *g_L = NULL;

static b2World *g_W = NULL;

static LTfloat g_screen_w = 1024.0f;
static LTfloat g_screen_h = 768.0f;

static LTfloat g_viewport_x1 = -15.0f;
static LTfloat g_viewport_y1 = -10.0f;
static LTfloat g_viewport_x2 = 15.0f;
static LTfloat g_viewport_y2 = 10.0f;

/************************* Graphics **************************/

static int lt_SetViewPort(lua_State *L) {
    g_viewport_x1 = (LTfloat)luaL_checknumber(L, 1);
    g_viewport_y1 = (LTfloat)luaL_checknumber(L, 2);
    g_viewport_x2 = (LTfloat)luaL_checknumber(L, 3);
    g_viewport_y2 = (LTfloat)luaL_checknumber(L, 4);
    ltSetViewPort(g_viewport_x1, g_viewport_y1, g_viewport_x2, g_viewport_y2);
    return 0;
}

static int lt_SetColor(lua_State *L) {
    int num_args = lua_gettop(L);
    LTfloat r = (LTfloat)luaL_checknumber(L, 1);
    LTfloat g = (LTfloat)luaL_checknumber(L, 2);
    LTfloat b = (LTfloat)luaL_checknumber(L, 3);
    LTfloat a;
    if (num_args > 3) {
        a = (LTfloat)luaL_checknumber(L, 4);
    } else {
        a = 1.0f;
    }
    ltSetColor(r, g, b, a);
    return 0;
}

static int lt_Scale(lua_State *L) {
    int num_args = lua_gettop(L);
    LTfloat x = (LTfloat)luaL_checknumber(L, 1);
    LTfloat y;
    if (num_args > 1) {
        y = (LTfloat)luaL_checknumber(L, 2);
    } else {
        y = x;
    }
    LTfloat z;
    if (num_args > 2) {
        z = (LTfloat)luaL_checknumber(L, 3);
    } else {
        z = 1.0f;
    }
    ltScale(x, y, z);
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
    LTfloat x1 = (LTfloat)luaL_checknumber(L, 1);
    LTfloat y1 = (LTfloat)luaL_checknumber(L, 2);
    LTfloat x2 = (LTfloat)luaL_checknumber(L, 3);
    LTfloat y2 = (LTfloat)luaL_checknumber(L, 4);
    ltDrawRect(x1, y1, x2, y2);
    return 0;
}

static int lt_DrawEllipse(lua_State *L) {
    LTfloat x = (LTfloat)luaL_checknumber(L, 1);
    LTfloat y = (LTfloat)luaL_checknumber(L, 2);
    LTfloat rx = (LTfloat)luaL_checknumber(L, 3);
    LTfloat ry = (LTfloat)luaL_checknumber(L, 4);
    ltDrawEllipse(x, y, rx, ry);
    return 0;
}

/************************* Images **************************/

// User must free with DeleteImagePacker.
static int lt_ImagePacker(lua_State *L) {
    int size = (int)luaL_checkinteger(L, 1);
    LTImagePacker *packer = new LTImagePacker(0, 0, size, size);
    LTImagePacker **ud = (LTImagePacker **)lua_newuserdata(L, sizeof(LTImagePacker **));
    *ud = packer;
    luaL_newmetatable(L, "pkr");
    lua_setmetatable(L, -2);
    return 1;
}

static int lt_ImagePackerSize(lua_State *L) {
    LTImagePacker **packer = (LTImagePacker**)luaL_checkudata(L, 1, "pkr");
    lua_pushinteger(L, (*packer)->size());
    return 1;
}

static int lt_DeleteImagePacker(lua_State *L) {
    LTImagePacker **packer = (LTImagePacker**)luaL_checkudata(L, 1, "pkr");
    delete *packer;
    *packer = NULL;
    return 0;
}

// User must free with DeleteImagesInPacker.
static int lt_ReadImage(lua_State *L) {
    const char *file = luaL_checkstring(L, 1);
    LTImageBuffer *buf = ltReadImage(file);
    LTImageBuffer **ud = (LTImageBuffer **)lua_newuserdata(L, sizeof(LTImageBuffer **));
    *ud = buf;
    luaL_newmetatable(L, "imgbuf");
    lua_setmetatable(L, -2);
    return 1;
}

static int lt_PackImage(lua_State *L) {
    LTImagePacker **packer = (LTImagePacker**)luaL_checkudata(L, 1, "pkr");
    LTImageBuffer **img_buf = (LTImageBuffer**)luaL_checkudata(L, 2, "imgbuf");
    if (ltPackImage(*packer, *img_buf)) {
        lua_pushboolean(L, 1);
    } else {
        lua_pushboolean(L, 0);
    }
    return 1;
}

static int lt_CreateAtlasTexture(lua_State *L) {
    LTImagePacker **packer = (LTImagePacker**)luaL_checkudata(L, 1, "pkr");
    LTtexture tex = ltCreateAtlasTexture(*packer);
    lua_pushinteger(L, (lua_Integer)tex); // XXX relies on lua_Integer being compatible with GLuint.
    return 1;
}

static int lt_DeleteTexture(lua_State *L) {
    LTtexture tex = (LTtexture)luaL_checkinteger(L, 1);
    ltDeleteTexture(tex);
    return 0;
}

static int img_DrawBottomLeft(lua_State *L) {
    LTImage **img = (LTImage**)luaL_checkudata(L, 1, "img");
    (*img)->drawBottomLeft();
    return 0;
}

static void push_image(lua_State *L, LTImage *img) {
    LTImage **ud = (LTImage **)lua_newuserdata(L, sizeof(LTImage **));
    *ud = img;
    if (luaL_newmetatable(L, "img")) {
        lua_createtable(L, 0, 16);
            lua_pushcfunction(L, img_DrawBottomLeft);
            lua_setfield(L, -2, "DrawBottomLeft");
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);
}

static void add_packer_images_to_lua_table(lua_State *L, int table, int w, int h, LTImagePacker *packer, LTtexture atlas) {
    char img_name[128];
    int len;
    const char *file;
    if (packer->occupant != NULL) {
        LTImage *img = new LTImage(atlas, w, h, packer);
        file = packer->occupant->file;
        len = strlen(file);
        if (len <= 4) {
            ltAbort("PNG file name too short: %s.", file);
        }
        if (len > 120) {
            ltAbort("PNG file name too long: %s.", file);
        }
        if (strcmp(".png", file + len - 4) != 0) {
            ltAbort("File %s does not end in .png", file);
        }
        strncpy(img_name, file, len - 4); // Remove ".png" suffix.
        img_name[len - 4] = '\0';
        push_image(L, img);
        lua_setfield(L, table, img_name);
        add_packer_images_to_lua_table(L, table, w, h, packer->lo_child, atlas);
        add_packer_images_to_lua_table(L, table, w, h, packer->hi_child, atlas);
    }
}

static int lt_AddPackerImagesToTable(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    LTImagePacker **packer = (LTImagePacker**)luaL_checkudata(L, 2, "pkr");
    LTtexture atlas = (LTtexture)luaL_checkinteger(L, 3);
    add_packer_images_to_lua_table(L, 1, (*packer)->width, (*packer)->height, *packer, atlas);
    return 0;
}

static int lt_DeleteImage(lua_State *L) {
    LTImage **img = (LTImage**)luaL_checkudata(L, 1, "img");
    delete *img;
    *img = NULL;
    return 0;
}

static int lt_DeleteImagesInPacker(lua_State *L) {
    LTImagePacker **packer = (LTImagePacker**)luaL_checkudata(L, 1, "pkr");
    (*packer)->deleteOccupants();
    return 0;
}

/************************* Box2D **************************/

static int lt_DoWorldStep(lua_State *L) {
    g_W->Step(1.0f / 60.0f, 10, 8);
    return 0;
}

static int lt_SetGravity(lua_State *L) {
    LTfloat x = (LTfloat)luaL_checknumber(L, 1);
    LTfloat y = (LTfloat)luaL_checknumber(L, 2);
    g_W->SetGravity(b2Vec2(x, y));
    return 0;
}

static b2Body **check_body(lua_State *L, int narg) {
    return (b2Body **)luaL_checkudata(L, narg, "bdy");
}

static int bdy_Destroy(lua_State *L) {
    b2Body** body = check_body(L, 1);
    if (*body != NULL) {
        g_W->DestroyBody(*body);
        *body = NULL;
    }
    return 0;
}

static int bdy_IsDestroyed(lua_State *L) {
    b2Body** body = check_body(L, 1);
    lua_pushboolean(L, *body == NULL);
    return 1;
}

static int bdy_ApplyForce(lua_State *L) {
    int num_args = lua_gettop(L);
    b2Body** body = check_body(L, 1);
    if (*body != NULL) {
        b2Vec2 force;
        b2Vec2 pos;
        force.x = luaL_checknumber(L, 2);
        force.y = (LTfloat)luaL_checknumber(L, 3);
        if (num_args >= 5) {
            pos.x = (LTfloat)luaL_checknumber(L, 4);
            pos.y = (LTfloat)luaL_checknumber(L, 5);
        } else {
            pos = (*body)->GetWorldCenter();
        }
        (*body)->ApplyForce(force, pos);
    }
    return 0;
}

static int bdy_ApplyTorque(lua_State *L) {
    b2Body** body = check_body(L, 1);
    if (*body != NULL) {
        (*body)->ApplyTorque(luaL_checknumber(L, 2));
    }
    return 0;
}

static int bdy_GetAngle(lua_State *L) {
    b2Body** body = check_body(L, 1);
    if (*body != NULL) {
        lua_pushnumber(L, (*body)->GetAngle());
        return 1;
    }
    return 0;
}

// XXX This doesn't currently work.
/*
static int bdy_SetAngle(lua_State *L) {
    b2Body** body = check_body(L, 1);
    if (*body != NULL) {
        (*body)->SetTransform((*body)->GetPosition(), luaL_checknumber(L, 2));
    }
    return 0;
}
*/

static int bdy_AddRect(lua_State *L) {
    int num_args = lua_gettop(L);
    b2Body** body = check_body(L, 1);
    if (*body != NULL) {
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
        (*body)->CreateFixture(&poly, density);
    }
    return 0;
}

static int bdy_DrawShapes(lua_State *L) {
    b2Body** body = check_body(L, 1);
    if (*body != NULL) {
        ltPushMatrix();
        b2Vec2 pos = (*body)->GetPosition();
        ltTranslate(pos.x, pos.y);
        ltRotate((*body)->GetAngle() * LT_DEGREES_PER_RADIAN);
        b2Fixture *fixture = (*body)->GetFixtureList();
        while (fixture != NULL) {
            b2Shape *shape = fixture->GetShape();
            switch (shape->m_type) {
		case b2Shape::e_unknown:
                    break;
		case b2Shape::e_circle:
                    break;
                case b2Shape::e_polygon: {
                    b2PolygonShape *poly = (b2PolygonShape *)shape;
                    ltDrawPoly((LTfloat *)poly->m_vertices, poly->m_vertexCount);
                    break;
                }
		case b2Shape::e_typeCount:
                    break;
            }
            fixture = fixture->GetNext();
        }
        ltPopMatrix();
    }
    return 0;
}

static void push_body(lua_State *L, b2BodyDef *def) {
    b2Body **ud = (b2Body **)lua_newuserdata(L, sizeof(b2Body **));
    b2Body *body = g_W->CreateBody(def);
    *ud = body;
    if (luaL_newmetatable(L, "bdy")) {
        lua_createtable(L, 0, 16);
            lua_pushcfunction(L, bdy_Destroy);
            lua_setfield(L, -2, "Destroy");
            lua_pushcfunction(L, bdy_IsDestroyed);
            lua_setfield(L, -2, "IsDestroyed");

            lua_pushcfunction(L, bdy_ApplyForce);
            lua_setfield(L, -2, "ApplyForce");
            lua_pushcfunction(L, bdy_ApplyTorque);
            lua_setfield(L, -2, "ApplyTorque");

            lua_pushcfunction(L, bdy_GetAngle);
            lua_setfield(L, -2, "GetAngle");

            lua_pushcfunction(L, bdy_AddRect);
            lua_setfield(L, -2, "AddRect");
            lua_pushcfunction(L, bdy_DrawShapes);
            lua_setfield(L, -2, "DrawShapes");
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);
}

static int lt_StaticBody(lua_State *L) {
    b2BodyDef def;
    def.type = b2_staticBody;
    push_body(L, &def);
    return 1;
}

static int lt_DynamicBody(lua_State *L) {
    int num_args = lua_gettop(L);
    LTfloat x = (LTfloat)luaL_checknumber(L, 1);
    LTfloat y = (LTfloat)luaL_checknumber(L, 2);
    LTfloat angle = 0.0f;
    if (num_args > 2) {
        angle = (LTfloat)luaL_checknumber(L, 3);
    }
    b2BodyDef def;
    def.type = b2_dynamicBody;
    def.position.Set(x, y);
    def.angle = angle;
    push_body(L, &def);
    return 1;
}

/************************************************************/

static const luaL_Reg ltlib[] = {
    {"SetViewPort",             lt_SetViewPort},
    {"SetColor",                lt_SetColor},
    {"Scale",                   lt_Scale},
    {"DrawUnitSquare",          lt_DrawUnitSquare},
    {"DrawUnitCircle",          lt_DrawUnitCircle},
    {"DrawRect",                lt_DrawRect},
    {"DrawEllipse",             lt_DrawEllipse},

    {"ImagePacker",             lt_ImagePacker},
    {"ImagePackerSize",         lt_ImagePackerSize},
    {"DeleteImagePacker",       lt_DeleteImagePacker},
    {"ReadImage",               lt_ReadImage},
    {"PackImage",               lt_PackImage},
    {"CreateAtlasTexture",      lt_CreateAtlasTexture},
    {"DeleteTexture",           lt_DeleteTexture},
    {"AddPackerImagesToTable",  lt_AddPackerImagesToTable},
    {"DeleteImage",             lt_DeleteImage},
    {"DeleteImagesInPacker",    lt_DeleteImagesInPacker},

    {"DoWorldStep",             lt_DoWorldStep},
    {"SetGravity",              lt_SetGravity},

    {"StaticBody",              lt_StaticBody},
    {"DynamicBody",             lt_DynamicBody},

    {NULL, NULL}
};

static void check_status(int status, bool abort) {
    if (status && !lua_isnil(g_L, -1)) {
        const char *msg = lua_tostring(g_L, -1);
        if (msg == NULL) msg = "(error object is not a string)";
        fprintf(stderr, "%s\n", msg);
        lua_pop(g_L, 1);
        if (abort) {
            ltHarnessQuit();
        }
    }
}

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
        check_status(lua_pcall(g_L, 0, 0, 0), true);
    }
}

void ltLuaSetup(const char *file) {
    g_W = new b2World(b2Vec2(0.0f, -10.0f), true);
    g_L = luaL_newstate();
    if (g_L == NULL) {
        fprintf(stderr, "Cannot create lua state: not enough memory.\n");
        exit(1);
    }
    lua_gc(g_L, LUA_GCSTOP, 0);  /* stop collector during library initialization */
    luaL_openlibs(g_L);
    luaL_register(g_L, "lt", ltlib);
    lua_gc(g_L, LUA_GCRESTART, 0);
    check_status(luaL_loadfile(g_L, file), true);
    check_status(lua_pcall(g_L, 0, 0, 0), true);
}

void ltLuaTeardown() {
    if (g_L != NULL) {
        lua_close(g_L);
    }
    delete g_W;
    g_W = NULL;
}

void ltLuaAdvance() {
    if (g_L != NULL) {
        call_lt_func("Advance");
    }
}

void ltLuaRender() {
    if (g_L != NULL) {
        call_lt_func("Render");
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
        case LT_KEY_PLUS: return "+"; 
        case LT_KEY_MINUS: return "-"; 
        case LT_KEY_TICK: return "`"; 
        case LT_KEY_UNKNOWN: return "unknown";
    }
    return "";
}

void ltLuaKeyDown(LTKey key) {
    if (g_L != NULL && push_lt_func("KeyDown")) {
        const char *str = lt_key_str(key);
        lua_pushstring(g_L, str);
        check_status(lua_pcall(g_L, 1, 0, 0), true);
    }
}

void ltLuaKeyUp(LTKey key) {
    if (g_L != NULL && push_lt_func("KeyUp")) {
        const char *str = lt_key_str(key);
        lua_pushstring(g_L, str);
        check_status(lua_pcall(g_L, 1, 0, 0), true);
    }
}

static LTfloat viewport_x(LTfloat screen_x) {
    return g_viewport_x1 + (screen_x / g_screen_w) * (g_viewport_x2 - g_viewport_x1);
}

static LTfloat viewport_y(LTfloat screen_y) {
    return g_viewport_y2 - (screen_y / g_screen_h) * (g_viewport_y2 - g_viewport_y1);
}

void ltLuaMouseDown(int button, LTfloat x, LTfloat y) {
    if (g_L != NULL && push_lt_func("MouseDown")) {
        lua_pushinteger(g_L, button);
        lua_pushnumber(g_L, viewport_x(x));
        lua_pushnumber(g_L, viewport_y(y));
        check_status(lua_pcall(g_L, 3, 0, 0), true);
    }
}

void ltLuaMouseUp(int button, LTfloat x, LTfloat y) {
    if (g_L != NULL && push_lt_func("MouseUp")) {
        lua_pushinteger(g_L, button);
        lua_pushnumber(g_L, viewport_x(x));
        lua_pushnumber(g_L, viewport_y(y));
        check_status(lua_pcall(g_L, 3, 0, 0), true);
    }
}

void ltLuaMouseMove(LTfloat x, LTfloat y) {
    if (g_L != NULL && push_lt_func("MouseMove")) {
        lua_pushnumber(g_L, viewport_x(x));
        lua_pushnumber(g_L, viewport_y(y));
        check_status(lua_pcall(g_L, 2, 0, 0), true);
    }
}

void ltLuaResizeWindow(LTfloat w, LTfloat h) {
    g_screen_w = w;
    g_screen_h = h;
}
