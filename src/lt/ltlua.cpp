/* Copyright (C) 2010 Ian MacLarty */

#include <string.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "Box2D/Box2D.h"
#include "ltgraphics.h"
#include "ltharness.h"
#include "ltlua.h"
#include "ltphysics.h"

static lua_State *g_L = NULL;

static LTfloat g_screen_w = 1024.0f;
static LTfloat g_screen_h = 768.0f;

static LTfloat g_viewport_x1 = -15.0f;
static LTfloat g_viewport_y1 = -10.0f;
static LTfloat g_viewport_x2 = 15.0f;
static LTfloat g_viewport_y2 = 10.0f;

/************************* Objects **************************/

static LTObject* get_object(lua_State *L, int index, LTType type) {
    LTObject **ud = (LTObject**)lua_touserdata(L, index);
    if (ud == NULL) {
        luaL_typerror(L, index, ltTypeName(type));
    }
    if (*ud == NULL) {
        luaL_error(L, "Userdata is NULL.");
    }
    LTObject *o = *ud;
    if (!o->hasType(type)) {
        luaL_typerror(L, index, ltTypeName(type));
    }
    return o;
}

static int release_object(lua_State *L) {
    LTObject **ud = (LTObject**)lua_touserdata(L, 1);
    if (*ud != NULL) {
        (*ud)->release();
        *ud = NULL;
    }
    return 0;
}

static int set_object_field(lua_State *L) {
    const char *fname;
    LTfloat val;
    LTObject **ud = (LTObject**)lua_touserdata(L, 1);
    if (lua_isstring(L, 2)) {
        fname = luaL_checkstring(L, 2);
        val = (LTfloat)luaL_checknumber(L, 3);
        if (*ud != NULL) {
            LTfloat *f = (*ud)->field_ptr(fname);
            if (f != NULL) {
                *f = val;
            }
        }
    } else {
        // Allow multiple fields to be set with a table.
        lua_pushnil(L);
        while (lua_next(L, 2) != 0) {
            fname = luaL_checkstring(L, -2);
            val = (LTfloat)luaL_checknumber(L, -1);
            if (*ud != NULL) {
                LTfloat *f = (*ud)->field_ptr(fname);
                if (f != NULL) {
                    *f = val;
                }
            }
            lua_pop(L, 1);
        }
    }
    return 0;
}

static int get_object_field(lua_State *L) {
    LTObject **ud = (LTObject**)lua_touserdata(L, 1);
    const char *fname = luaL_checkstring(L, 2);
    if (*ud != NULL) {
        LTfloat *f = (*ud)->field_ptr(fname);
        if (f != NULL) {
            lua_pushnumber(L, *f);
            return 1;
        }
    }
    return 0;
}

static void push_object(lua_State *L, LTObject *obj, const luaL_Reg *methods) {
    // We use a full userdata so we can get Lua GC finalize events.
    LTObject **ud = (LTObject **)lua_newuserdata(L, sizeof(LTObject *));
    *ud = obj;
    // Count all Lua references to the object as one reference.
    obj->retain();
    if (luaL_newmetatable(L, obj->typeName())) {
        if (methods != NULL) {
            lua_newtable(L);
                // All objects get a Release method that can be used to
                // manually free them.
                lua_pushcfunction(L, release_object);
                lua_setfield(L, -2, "Release");

                // Field access functions.
                lua_pushcfunction(L, get_object_field);
                lua_setfield(L, -2, "Get");
                lua_pushcfunction(L, set_object_field);
                lua_setfield(L, -2, "Set");

                luaL_register(L, NULL, methods);
            lua_setfield(L, -2, "__index");
        }
        // Decrement reference count when Lua no longer references the object.
        // This will have no effect if the object was released using the Release method.
        lua_pushcfunction(L, release_object);
        lua_setfield(L, -2, "__gc");
    }
    lua_setmetatable(L, -2);
}

/************************* Graphics **************************/

static int lt_SetViewPort(lua_State *L) {
    g_viewport_x1 = (LTfloat)luaL_checknumber(L, 1);
    g_viewport_y1 = (LTfloat)luaL_checknumber(L, 2);
    g_viewport_x2 = (LTfloat)luaL_checknumber(L, 3);
    g_viewport_y2 = (LTfloat)luaL_checknumber(L, 4);
    ltSetViewPort(g_viewport_x1, g_viewport_y1, g_viewport_x2, g_viewport_y2);
    return 0;
}

static int lt_PushTint(lua_State *L) {
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
    ltPushTint(r, g, b, a);
    return 0;
}

static int lt_PopTint(lua_State *L) {
    ltPopTint();
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

static int lt_Translate(lua_State *L) {
    int num_args = lua_gettop(L);
    LTfloat x = (LTfloat)luaL_checknumber(L, 1);
    LTfloat y = (LTfloat)luaL_checknumber(L, 2);
    LTfloat z;
    if (num_args > 2) {
        z = (LTfloat)luaL_checknumber(L, 3);
    } else {
        z = 0.0f;
    }
    ltTranslate(x, y, z);
    return 0;
}

static int lt_Rotate(lua_State *L) {
    int num_args = lua_gettop(L);
    LTfloat theta = (LTdegrees)luaL_checknumber(L, 1);
    LTfloat x;
    if (num_args > 1) {
        x = (LTfloat)luaL_checknumber(L, 2);
    } else {
        x = 0.0f;
    }
    LTfloat y;
    if (num_args > 2) {
        y = (LTfloat)luaL_checknumber(L, 3);
    } else {
        y = 0.0f;
    }
    LTfloat z;
    if (num_args > 3) {
        z = (LTfloat)luaL_checknumber(L, 4);
    } else {
        z = 1.0f;
    }
    ltRotate(theta, x, y, z);
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

/************************* Props **************************/

static int prop_Draw(lua_State *L) {
    LTProp *prop = (LTProp*)get_object(L, 1, LT_TYPE_PROP);
    prop->draw();
    return 0;
}

static int scene_Insert(lua_State *L) {
    LTScene *scene = (LTScene*)get_object(L, 1, LT_TYPE_SCENE);
    LTProp *prop = (LTProp*)get_object(L, 2, LT_TYPE_PROP);
    LTfloat depth = luaL_checknumber(L, 3);
    scene->insert(prop, depth);
    return 0;
}

static int scene_Remove(lua_State *L) {
    LTScene *scene = (LTScene*)get_object(L, 1, LT_TYPE_SCENE);
    LTProp *prop = (LTProp*)get_object(L, 2, LT_TYPE_PROP);
    scene->remove(prop);
    return 0;
}

static const luaL_Reg scene_methods[] = {
    {"Draw",            prop_Draw},
    {"Insert",          scene_Insert},
    {"Remove",          scene_Remove},

    {NULL, NULL}
};

static int lt_Scene(lua_State *L) {
    LTScene *scene = new LTScene();
    push_object(L, scene, scene_methods);
    return 1;
}

static const luaL_Reg prop_methods[] = {
    {"Draw",            prop_Draw},

    {NULL, NULL}
};

static int lt_Tinter(lua_State *L) {
    int num_args = lua_gettop(L);
    LTProp *target = (LTProp *)get_object(L, 1, LT_TYPE_PROP);
    LTfloat r = (LTfloat)luaL_checknumber(L, 2);
    LTfloat g = (LTfloat)luaL_checknumber(L, 3);
    LTfloat b = (LTfloat)luaL_checknumber(L, 4);
    LTfloat a = 1.0f;
    if (num_args > 4) {
        a = (LTfloat)luaL_checknumber(L, 5);
    }
    LTTinter *tinter = new LTTinter(r, g, b, a, target);
    push_object(L, tinter, prop_methods);
    return 1;
}

static int lt_Line(lua_State *L) {
    LTfloat x1 = (LTfloat)luaL_checknumber(L, 1);
    LTfloat y1 = (LTfloat)luaL_checknumber(L, 2);
    LTfloat x2 = (LTfloat)luaL_checknumber(L, 3);
    LTfloat y2 = (LTfloat)luaL_checknumber(L, 4);
    LTLine *line = new LTLine(x1, y1, x2, y2);
    push_object(L, line, prop_methods);
    return 1;
}

static int lt_Triangle(lua_State *L) {
    LTfloat x1 = (LTfloat)luaL_checknumber(L, 1);
    LTfloat y1 = (LTfloat)luaL_checknumber(L, 2);
    LTfloat x2 = (LTfloat)luaL_checknumber(L, 3);
    LTfloat y2 = (LTfloat)luaL_checknumber(L, 4);
    LTfloat x3 = (LTfloat)luaL_checknumber(L, 5);
    LTfloat y3 = (LTfloat)luaL_checknumber(L, 6);
    LTTriangle *triangle = new LTTriangle(x1, y1, x2, y2, x3, y3);
    push_object(L, triangle, prop_methods);
    return 1;
}

/************************* Images **************************/

static int img_Draw(lua_State *L) {
    LTImage *img = (LTImage*)get_object(L, 1, LT_TYPE_IMAGE);
    img->draw();
    return 0;
}

static const luaL_Reg img_methods[] = {
    {"Draw",                    img_Draw},

    {NULL, NULL}
};

static void add_packer_images_to_lua_table(lua_State *L, int w, int h, LTImagePacker *packer, LTAtlas *atlas) {
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
        push_object(L, img, img_methods);
        lua_setfield(L, -2, img_name);
        add_packer_images_to_lua_table(L, w, h, packer->lo_child, atlas);
        add_packer_images_to_lua_table(L, w, h, packer->hi_child, atlas);
    }
}

// Load images in 1st argument (an array) and return a table
// indexed by image name (without the .png suffix).
// The second argument is the size of the atlasses to generate
// (1024 by default).
static int lt_LoadImages(lua_State *L) {
    int num_args = lua_gettop(L);
    int size = 1024;
    if (num_args > 1) {
        size = (int)luaL_checkinteger(L, 2);
    }
    lua_newtable(L); // The table to be returned.
    LTImagePacker *packer = new LTImagePacker(0, 0, size, size);
    int atlas_num = 1;
    int i = 1;
    do {
        lua_pushinteger(L, i);
        lua_gettable(L, 1);
        if (lua_isnil(L, -1)) {
            // We've reached the end of the array.
            lua_pop(L, 1);
            break;
        }
        const char* file = lua_tostring(L, -1);
        lua_pop(L, 1);
        if (file == NULL) {
            return luaL_error(L, "Expecting an array of strings.");
        }
        
        LTImageBuffer *buf = ltReadImage(file);
        if (!ltPackImage(packer, buf)) {
            // Packer full, so generate an atlas.
            LTAtlas *atlas = new LTAtlas(packer);
            add_packer_images_to_lua_table(L, packer->width, packer->height, packer, atlas);
            packer->deleteOccupants();
            atlas_num++;

            if (!ltPackImage(packer, buf)) {
                return luaL_error(L, "Image %s is too large.", file);
            }
        }

        i++;
    } while (true);

    // Pack any images left in packer into a new texture.
    if (packer->size() > 0) {
        LTAtlas *atlas = new LTAtlas(packer);
        add_packer_images_to_lua_table(L, packer->width, packer->height, packer, atlas);
        packer->deleteOccupants();
        atlas_num++;
    }
        
    delete packer;

    return 1;
}

/************************* Box2D **************************/

static int fixture_ContainsPoint(lua_State *L) {
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

static const luaL_Reg fixture_methods[] = {
    //{"Destroy",             fixture_Destroy},
    //{"IsDestroyed",         fixture_IsDestroyed},
    {"Draw",                prop_Draw},
    {"ContainsPoint",       fixture_ContainsPoint},

    {NULL, NULL}
};

static int wld_Step(lua_State *L) {
    int num_args = lua_gettop(L);
    LTWorld *world = (LTWorld*)get_object(L, 1, LT_TYPE_WORLD);
    LTfloat time_step = luaL_checknumber(L, 2);
    int velocity_iterations = 10;
    int position_iterations = 8;
    if (num_args > 2) {
        velocity_iterations = luaL_checkinteger(L, 3);
    }
    if (num_args > 3) {
        position_iterations = luaL_checkinteger(L, 4);
    }
    world->world->Step(time_step, velocity_iterations, position_iterations);
    return 0;
}

static int wld_SetGravity(lua_State *L) {
    LTWorld *world = (LTWorld*)get_object(L, 1, LT_TYPE_WORLD);
    LTfloat x = (LTfloat)luaL_checknumber(L, 2);
    LTfloat y = (LTfloat)luaL_checknumber(L, 3);
    world->world->SetGravity(b2Vec2(x, y));
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
        lua_pushinteger(L, i);
        LTFixture *f = (LTFixture*)fixture->GetUserData();
        push_object(L, f, fixture_methods);
        lua_settable(L, -3);
        i++;
        return true;
    }
};

static int wld_QueryBox(lua_State *L) {
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

static int bdy_Destroy(lua_State *L) {
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    body->destroy();
    return 0;
}

static int bdy_IsDestroyed(lua_State *L) {
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
   lua_pushboolean(L, body->body == NULL);
    return 1;
}

static int bdy_ApplyForce(lua_State *L) {
    int num_args = lua_gettop(L);
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

static int bdy_ApplyTorque(lua_State *L) {
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        body->body->ApplyTorque(luaL_checknumber(L, 2));
    }
    return 0;
}

static int bdy_GetAngle(lua_State *L) {
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        lua_pushnumber(L, body->body->GetAngle() * LT_DEGREES_PER_RADIAN);
        return 1;
    }
    return 0;
}

static int bdy_SetAngle(lua_State *L) {
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        body->body->SetTransform(body->body->GetPosition(), luaL_checknumber(L, 2) * LT_RADIANS_PER_DEGREE);
    }
    return 0;
}

static int bdy_GetPosition(lua_State *L) {
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        b2Vec2 pos = body->body->GetPosition();
        lua_pushnumber(L, pos.x);
        lua_pushnumber(L, pos.y);
        return 2;
    }
    return 0;
}

static int bdy_SetAngularVelocity(lua_State *L) {
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        body->body->SetAngularVelocity(luaL_checknumber(L, 2) * LT_RADIANS_PER_DEGREE);
    }
    return 0;
}

static int bdy_AddRect(lua_State *L) {
    int num_args = lua_gettop(L);
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
        new LTFixture(body, &fixtureDef);
    }
    return 0;
}

static int bdy_AddTriangle(lua_State *L) {
    int num_args = lua_gettop(L);
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
                lua_pushboolean(L, 0);
                return 1;
            }
        }
        poly.Set(vertices, 3);
        b2FixtureDef fixtureDef;
        fixtureDef.density = density;
        fixtureDef.shape = &poly;
        new LTFixture(body, &fixtureDef);
        lua_pushboolean(L, 1);
        return 1;
    }
    return 0;
}

static int bdy_Fixture(lua_State *L) {
    LTBody *body = (LTBody*)get_object(L, 1, LT_TYPE_BODY);
    if (body->body != NULL) {
        LTObject *obj = get_object(L, 1, LT_TYPE_OBJECT);
        if (obj->type == LT_TYPE_TRIANGLE) {
            LTTriangle *t = (LTTriangle*)obj;
            b2PolygonShape poly;
            b2Vec2 vertices[3];
            vertices[0].Set(t->x1, t->y1);
            vertices[1].Set(t->x2, t->y2);
            vertices[2].Set(t->x3, t->y3);
            if (!ltCheckB2Poly(vertices, 3)) {
                vertices[2] = vertices[0];
                vertices[0].Set(t->x3, t->y3);
                if (!ltCheckB2Poly(vertices, 3)) {
                    return 0;
                }
            }
            poly.Set(vertices, 3);
            b2FixtureDef fixtureDef;
            fixtureDef.shape = &poly;
            LTFixture *f = new LTFixture(body, &fixtureDef);
            push_object(L, f, fixture_methods);
            return 1;
        }
        return luaL_error(L, "Expecting a shape.");
    }
    return 0;
}

static const luaL_Reg body_methods[] = {
    {"Destroy",             bdy_Destroy},
    {"IsDestroyed",         bdy_IsDestroyed},
    {"ApplyForce",          bdy_ApplyForce},
    {"ApplyTorque",         bdy_ApplyTorque},
    {"GetAngle",            bdy_GetAngle},
    {"SetAngle",            bdy_SetAngle},
    {"GetPosition",         bdy_GetPosition},
    {"AddRect",             bdy_AddRect},
    {"AddTriangle",         bdy_AddTriangle},
    {"Draw",                prop_Draw},
    {"SetAngularVelocity",  bdy_SetAngularVelocity},
    {"Fixture",             bdy_Fixture},

    {NULL, NULL}
};

static int wld_StaticBody(lua_State *L) {
    LTWorld *world = (LTWorld*)get_object(L, 1, LT_TYPE_WORLD);
    b2BodyDef def;
    def.type = b2_staticBody;
    LTBody *body = new LTBody(world, &def);
    push_object(L, body, body_methods);
    return 1;
}

static int wld_DynamicBody(lua_State *L) {
    int num_args = lua_gettop(L);
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
    push_object(L, body, body_methods);
    return 1;
}

static const luaL_Reg world_methods[] = {
    {"Step",             wld_Step},
    {"SetGravity",       wld_SetGravity},
    {"StaticBody",       wld_StaticBody},
    {"DynamicBody",      wld_DynamicBody},
    {"QueryBox",         wld_QueryBox},

    {NULL, NULL}
};

static int lt_World(lua_State *L) {
    LTWorld *world = new LTWorld(b2Vec2(0.0f, -10.0f), true);
    push_object(L, world, world_methods);
    return 1;
}

/************************************************************/

static const luaL_Reg ltlib[] = {
    {"SetViewPort",             lt_SetViewPort},
    {"PushTint",                lt_PushTint},
    {"PopTint",                 lt_PopTint},
    {"Scale",                   lt_Scale},
    {"Translate",               lt_Translate},
    {"Rotate",                  lt_Rotate},
    {"PushMatrix",              lt_PushMatrix},
    {"PopMatrix",               lt_PopMatrix},
    {"DrawUnitSquare",          lt_DrawUnitSquare},
    {"DrawUnitCircle",          lt_DrawUnitCircle},
    {"DrawRect",                lt_DrawRect},
    {"DrawEllipse",             lt_DrawEllipse},

    {"Scene",                   lt_Scene},
    {"Line",                    lt_Line},
    {"Triangle",                lt_Triangle},
    {"Tinter",                  lt_Tinter},

    {"LoadImages",              lt_LoadImages},

    {"World",                   lt_World},

    {NULL, NULL}
};

/************************************************************/

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
