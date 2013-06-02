/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltbox2d)

LTWorld::LTWorld() {
    world = new b2World(b2Vec2(0.0f, 0.0f));
    world->SetAllowSleeping(true);
    scale = 1.0f;
    debug = false;
}

LTWorld::~LTWorld() {
    delete world;
}

LTBody::LTBody(LTWorld *world, const b2BodyDef *def) {
    LTBody::world = world;
    body = world->world->CreateBody(def);
    body->SetUserData(this);
}

void LTBody::destroy() {
    if (body != NULL) { // NULL means the body was already destroyed.
        // Invalidate fixture wrappers.
        b2Fixture *f = body->GetFixtureList();
        while (f != NULL) {
            LTFixture *ud = (LTFixture*)f->GetUserData();
            ud->fixture = NULL;
            ud->body = NULL;
            f = f->GetNext();
        }
        // Invalidate joint wrappers.
        b2JointEdge *j = body->GetJointList();
        while (j != NULL) {
            LTJoint *ud = (LTJoint*)j->joint->GetUserData();
            ud->joint = NULL;
            j = j->next;
        }

        world->world->DestroyBody(body);
        world = NULL;
        body = NULL;
    }
}

void LTBody::draw() {
    if (body != NULL) {
        b2Vec2 pos = body->GetPosition();
        LTfloat scale = world->scale;
        LTbool debug = world->debug;

        if (debug) ltPushMatrix();

        if (child != NULL) {
            ltTranslate(pos.x * scale, pos.y * scale, 0.0f);
            ltRotate(body->GetAngle() * LT_DEGREES_PER_RADIAN, 0.0f, 0.0f, 1.0f);
            child->draw();
        }

        if (debug) {
            ltPopMatrix();
            ltScale(scale, scale, 0.0f);
            ltTranslate(pos.x, pos.y, 0.0f);
            ltRotate(body->GetAngle() * LT_DEGREES_PER_RADIAN, 0.0f, 0.0f, 1.0f);
            b2Fixture *fixture = body->GetFixtureList();
            while (fixture != NULL) {
                LTFixture *f = (LTFixture*)fixture->GetUserData();
                f->draw();
                fixture = fixture->GetNext();
            }
        }
    }
}

//bool LTBody::containsPoint(LTfloat x, LTfloat y) {
//    b2Vec2 p = b2Vec2(x, y);
//    b2Fixture *fixture = body->GetFixtureList();
//    while (fixture != NULL) {
//        if (fixture->TestPoint(p)) {
//            return true;
//        }
//        fixture = fixture->GetNext();
//    }
//    return false;
//}

bool LTBody::inverse_transform(LTfloat *x, LTfloat *y) {
    b2Body *b = body;
    if (b != NULL) {
        LTfloat angle = b->GetAngle();
        b2Vec2 pos = b->GetPosition();
        *x = *x - pos.x;
        *y = *y - pos.y;
        LTfloat x1, y1;
        LTfloat s = sinf(angle);
        LTfloat c = cosf(angle);
        x1 = c * (*x) - s * (*y);
        y1 = s * (*x) + c * (*y);
        *x = x1;
        *y = y1;
        return true;
    } else {
        return false;
    }
}

LTFixture::LTFixture(LTBody *body, const b2FixtureDef *def) {
    LTFixture::body = body;
    if (body->body != NULL) {
        fixture = body->body->CreateFixture(def);
        fixture->SetUserData(this);
    } else {
        // User tried to add fixture to destroyed body.
        fixture = NULL;
    }
}

void LTFixture::destroy() {
    if (fixture != NULL) { // NULL means the fixture was already destroyed.
        assert(body->body != NULL);
        body->body->DestroyFixture(fixture);
        fixture = NULL;
        body = NULL;
    }
}

void LTFixture::draw() {
    if (fixture != NULL) {
        b2Shape *shape = fixture->GetShape();
        switch (shape->m_type) {
            case b2Shape::e_edge:
                break;
            case b2Shape::e_chain: {
                b2ChainShape *chain = (b2ChainShape *)shape;
                ltPushTint(1.0f, 1.0f, 1.0f, 1.0f);
                ltDrawLineStrip((LTfloat *)chain->m_vertices, chain->m_count);
                ltPopTint();
                break;
            }
            case b2Shape::e_circle: {
                b2CircleShape *circle = (b2CircleShape *)shape;
                ltPushMatrix();
                ltPushTint(1.0f, 1.0f, 1.0f, 0.5f);
                ltTranslate(circle->m_p.x, circle->m_p.y, 0.0f);
                ltScale(circle->m_radius, circle->m_radius, 1.0f);
                ltDrawUnitCircle();
                ltPopTint();
                ltPopMatrix();
                break;
            }
            case b2Shape::e_polygon: {
                b2PolygonShape *poly = (b2PolygonShape *)shape;
                ltPushTint(1.0f, 1.0f, 1.0f, 0.5f);
                ltDrawPoly((LTfloat *)poly->m_vertices, poly->m_vertexCount);
                ltPopTint();
                for (int i = 0; i < poly->m_vertexCount - 1; i++) {
                    ltVertexPointer(2, LT_VERT_DATA_TYPE_FLOAT, 0, &poly->m_vertices[i]);
                    ltDrawArrays(LT_DRAWMODE_LINE_STRIP, 0, 2);
                }
                b2Vec2 final_line[2];
                final_line[0] = poly->m_vertices[poly->m_vertexCount - 1];
                final_line[1] = poly->m_vertices[0];
                ltVertexPointer(2, LT_VERT_DATA_TYPE_FLOAT, 0, final_line);
                ltDrawArrays(LT_DRAWMODE_LINE_STRIP, 0, 2);
                break;
            }
            case b2Shape::e_typeCount:
                break;
        }
    }
}

LTJoint::LTJoint(LTWorld *world, const b2JointDef *def) {
    LTJoint::world = world;
    LTJoint::joint = world->world->CreateJoint(def);
    LTJoint::joint->SetUserData(this);
}

void LTJoint::destroy() {
    if (joint != NULL) {
        world->world->DestroyJoint(joint);
        joint = NULL;
    }
}

LTBodyTracker::LTBodyTracker(LTBody *body, LTSceneNode *child,
    bool viewport_mode, bool track_rotation,
    LTfloat min_x, LTfloat max_x, LTfloat min_y, LTfloat max_y, LTfloat snap_to)
{
    LTBodyTracker::viewport_mode = viewport_mode;
    LTBodyTracker::track_rotation = track_rotation;
    LTBodyTracker::min_x = min_x;
    LTBodyTracker::max_x = max_x;
    LTBodyTracker::min_y = min_y;
    LTBodyTracker::max_y = max_y;
    LTBodyTracker::snap_to = snap_to;
    LTBodyTracker::body = body;
}

void LTBodyTracker::draw() {
    static LTfloat rmat[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    b2Body *b = body->body;
    if (b != NULL) {
        LTfloat scale = body->world->scale;
        const b2Transform b2t = b->GetTransform();
        LTfloat x = b2t.p.x * scale;
        LTfloat y = b2t.p.y * scale;
        if (snap_to > 0.0f) {
            x = roundf(x / snap_to) * snap_to;
            y = roundf(y / snap_to) * snap_to;
        }
        if (x < min_x) {
            x = min_x;
        } else if (x > max_x) {
            x = max_x;
        }
        if (y < min_y) {
            y = min_y;
        } else if (y > max_y) {
            y = max_y;
        }
        if (viewport_mode) {
            if (track_rotation) {
                rmat[0] = b2t.q.c;
                rmat[1] = -b2t.q.s;
                rmat[4] = b2t.q.s;
                rmat[5] = b2t.q.c;
                ltMultMatrix(rmat);
            }
            ltTranslate(-x, -y, 0.0f);
        } else {
            ltTranslate(x, y, 0.0f);
            if (track_rotation) {
                rmat[0] = b2t.q.c;
                rmat[1] = b2t.q.s;
                rmat[4] = -b2t.q.s;
                rmat[5] = b2t.q.c;
                ltMultMatrix(rmat);
            }
        }
        child->draw();
    }
}

bool LTBodyTracker::inverse_transform(LTfloat *x, LTfloat *y) {
    b2Body *b = body->body;
    if (b != NULL) {
        if (viewport_mode) {
            // XXX viewport mode NYI
            return false;
        }
        LTfloat angle = b->GetAngle();
        b2Vec2 pos = b->GetPosition();
        *x = *x - pos.x;
        *y = *y - pos.y;
        LTfloat x1, y1;
        LTfloat s = sinf(angle);
        LTfloat c = cosf(angle);
        x1 = c * (*x) - s * (*y);
        y1 = s * (*x) + c * (*y);
        *x = x1;
        *y = y1;
        return true;
    } else {
        return false;
    }
}

bool ltCheckB2Poly(const b2Vec2* vs, int32 count) {
    // This code copied from Box2D (b2PolygonShape.cpp, ComputeCentroid).

    if (count < 2) {
        return false;
    }
    if (count == 2) {
        return true;
    }

    b2Vec2 c;
    c.Set(0.0f, 0.0f);
    float32 area = 0.0f;
    b2Vec2 pRef(0.0f, 0.0f);

    const float32 inv3 = 1.0f / 3.0f;

    for (int32 i = 0; i < count; ++i) {
        b2Vec2 p1 = pRef;
        b2Vec2 p2 = vs[i];
        b2Vec2 p3 = i + 1 < count ? vs[i+1] : vs[0];

        b2Vec2 e1 = p2 - p1;
        b2Vec2 e2 = p3 - p1;

        float32 D = b2Cross(e1, e2);

        float32 triangleArea = 0.5f * D;
        area += triangleArea;

        c += triangleArea * inv3 * (p1 + p2 + p3);
    }

    return area > b2_epsilon;
}

//---------------------------------------------------

LTWorld *lt_expect_LTWorld(lua_State *L, int arg);
LTBody *lt_expect_LTBody(lua_State *L, int arg);
LTFixture *lt_expect_LTFixture(lua_State *L, int arg);
LTJoint *lt_expect_LTJoint(lua_State *L, int arg);

void *lt_alloc_LTBody(lua_State *L);
void *lt_alloc_LTFixture(lua_State *L);

static LTfloat get_world_gx(LTObject *obj) {
    LTWorld *w = (LTWorld*)obj;
    return w->world->GetGravity().x;
}

static LTfloat get_world_gy(LTObject *obj) {
    LTWorld *w = (LTWorld*)obj;
    return w->world->GetGravity().y;
}

static void set_world_gx(LTObject *obj, LTfloat val) {
    LTWorld *w = (LTWorld*)obj;
    b2Vec2 g = w->world->GetGravity();
    w->world->SetGravity(b2Vec2(val, g.y));
}

static void set_world_gy(LTObject *obj, LTfloat val) {
    LTWorld *w = (LTWorld*)obj;
    b2Vec2 g = w->world->GetGravity();
    w->world->SetGravity(b2Vec2(g.x, val));
}

static LTbool get_world_auto_clear_forces(LTObject *obj) {
    LTWorld *w = (LTWorld*)obj;
    return w->world->GetAutoClearForces();
}

static void set_world_auto_clear_forces(LTObject *obj, LTbool val) {
    LTWorld *w = (LTWorld*)obj;
    w->world->SetAutoClearForces(val);
}

static int world_step(lua_State *L) {
    int num_args = ltLuaCheckNArgs(L, 2); 
    LTWorld *world = lt_expect_LTWorld(L, 1);
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

static int new_body(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTWorld *world = lt_expect_LTWorld(L, 1);
    LTfloat scale = world->scale;

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

    lua_getfield(L, 2, "x");
    if (!lua_isnil(L, -1)) {
        body_def.position.x = luaL_checknumber(L, -1) / scale;
    }
    lua_pop(L, 1);
    lua_getfield(L, 2, "y");
    if (!lua_isnil(L, -1)) {
        body_def.position.y = luaL_checknumber(L, -1) / scale;
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "angle");
    if (!lua_isnil(L, -1)) {
        body_def.angle = LT_RADIANS_PER_DEGREE * luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "vx");
    if (!lua_isnil(L, -1)) {
        body_def.linearVelocity.x = luaL_checknumber(L, -1) / scale;
    }
    lua_pop(L, 1);
    lua_getfield(L, 2, "vy");
    if (!lua_isnil(L, -1)) {
        body_def.linearVelocity.y = luaL_checknumber(L, -1) / scale;
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

    LTBody *body = new (lt_alloc_LTBody(L)) LTBody(world, &body_def);
    body->world_ref = ltLuaAddRef(L, 1, -1); // Add reference from world to body.
    world->body_refs[body] = body->world_ref;
    ltLuaAddNamedRef(L, -1, 1, "world"); // Add reference from body to world.
    return 1;
}

static LTObject* get_body_world(LTObject* obj) {
    LTBody *b = (LTBody*)obj;
    return b->world;
}

static LTfloat get_body_x(LTObject *obj) {
    LTBody *b = (LTBody*)obj;
    if (b->body != NULL) {
        return b->body->GetPosition().x * b->world->scale;
    } else {
        return 0.0f;
    }
}

static void set_body_x(LTObject *obj, LTfloat val) {
    LTBody *b = (LTBody*)obj;
    if (b->body != NULL) {
        b2Vec2 pos = b->body->GetPosition();
        LTfloat angle = b->body->GetAngle();
        b->body->SetTransform(b2Vec2(val / b->world->scale, pos.y), angle);
        b->body->SetAwake(true);
    }
}

static LTfloat get_body_y(LTObject *obj) {
    LTBody *b = (LTBody*)obj;
    if (b->body != NULL) {
        return b->body->GetPosition().y * b->world->scale;
    } else {
        return 0.0f;
    }
}

static void set_body_y(LTObject *obj, LTfloat val) {
    LTBody *b = (LTBody*)obj;
    if (b->body != NULL) {
        b2Vec2 pos = b->body->GetPosition();
        LTfloat angle = b->body->GetAngle();
        b->body->SetTransform(b2Vec2(pos.x, val / b->world->scale), angle);
        b->body->SetAwake(true);
    }
}

static LTfloat get_body_angle(LTObject *obj) {
    LTBody *b = (LTBody*)obj;
    if (b->body != NULL) {
        return b->body->GetAngle() * LT_DEGREES_PER_RADIAN;
    } else {
        return 0.0f;
    }
}

static void set_body_angle(LTObject *obj, LTfloat val) {
    LTBody *b = (LTBody*)obj;
    if (b->body != NULL) {
        b2Vec2 pos = b->body->GetPosition();
        b->body->SetTransform(pos, val * LT_RADIANS_PER_DEGREE);
        b->body->SetAwake(true);
    }
}

static LTfloat get_body_vx(LTObject *obj) {
    LTBody *b = (LTBody*)obj;
    if (b->body != NULL) {
        return b->body->GetLinearVelocity().x * b->world->scale;
    } else {
        return 0.0f;
    }
}

static void set_body_vx(LTObject *obj, LTfloat val) {
    LTBody *b = (LTBody*)obj;
    if (b->body != NULL) {
        b2Vec2 v = b->body->GetLinearVelocity();
        b->body->SetLinearVelocity(b2Vec2(val / b->world->scale, v.y));
        b->body->SetAwake(true);
    }
}

static LTfloat get_body_vy(LTObject *obj) {
    LTBody *b = (LTBody*)obj;
    if (b->body != NULL) {
        return b->body->GetLinearVelocity().y * b->world->scale;
    } else {
        return 0.0f;
    }
}

static void set_body_vy(LTObject *obj, LTfloat val) {
    LTBody *b = (LTBody*)obj;
    if (b->body != NULL) {
        b2Vec2 v = b->body->GetLinearVelocity();
        b->body->SetLinearVelocity(b2Vec2(v.x, val / b->world->scale));
        b->body->SetAwake(true);
    }
}

static LTfloat get_body_angular_velocity(LTObject *obj) {
    LTBody *b = (LTBody*)obj;
    if (b->body != NULL) {
        return b->body->GetAngularVelocity();
    } else {
        return 0.0f;
    }
}

static LTbool get_body_destroyed(LTObject *obj) {
    LTBody *b = (LTBody*)obj;
    return b->body == NULL;
}

static void set_body_angular_velocity(LTObject *obj, LTfloat val) {
    LTBody *b = (LTBody*)obj;
    if (b->body != NULL) {
        b->body->SetAngularVelocity(val * LT_RADIANS_PER_DEGREE);
        b->body->SetAwake(true);
    }
}

static int body_apply_force(lua_State *L) {
    int num_args = ltLuaCheckNArgs(L, 3);
    LTBody *body = lt_expect_LTBody(L, 1);
    if (body->body != NULL) {
        b2Vec2 force;
        b2Vec2 pos;
        force.x = luaL_checknumber(L, 2);
        force.y = luaL_checknumber(L, 3);
        if (num_args >= 5) {
            pos.x = luaL_checknumber(L, 4);
            pos.y = luaL_checknumber(L, 5);
        } else {
            pos = body->body->GetWorldCenter();
        }
        body->body->ApplyForce(force, pos);
    }
    return 0;
}

static int body_apply_torque(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTBody *body = lt_expect_LTBody(L, 1);
    if (body->body != NULL) {
        body->body->ApplyTorque(luaL_checknumber(L, 2));
    }
    return 0;
}

static int body_apply_impulse(lua_State *L) {
    int num_args = ltLuaCheckNArgs(L, 3);
    LTBody *body = lt_expect_LTBody(L, 1);
    if (body->body != NULL) {
        b2Vec2 force;
        b2Vec2 pos;
        force.x = luaL_checknumber(L, 2);
        force.y = luaL_checknumber(L, 3);
        if (num_args >= 5) {
            pos.x = luaL_checknumber(L, 4);
            pos.y = luaL_checknumber(L, 5);
        } else {
            pos = body->body->GetWorldCenter();
        }
        body->body->ApplyLinearImpulse(force, pos);
    }
    return 0;
}

static int body_apply_angular_impulse(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTBody *body = lt_expect_LTBody(L, 1);
    if (body->body != NULL) {
        body->body->ApplyAngularImpulse(luaL_checknumber(L, 2));
    }
    return 0;
}

static int destroy_body(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTBody *body = lt_expect_LTBody(L, 1);
    b2Body *b = body->body;
    if (b != NULL) {
        LTWorld *world = body->world;
        body->destroy();
        ltLuaGetNamedRef(L, 1, "world"); // push world
        ltLuaDelRef(L, -1, body->world_ref); // Remove reference from world to body
                                             // so body can be GC'd.
        lua_pop(L, 1);
        body->world_ref = LUA_NOREF;
        world->body_refs.erase(body);
    }
    return 0;
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

static int new_polygon_fixture(lua_State *L) {
    int nargs = ltLuaCheckNArgs(L, 2);
    LTBody *body = lt_expect_LTBody(L, 1);
    if (body->body != NULL) {
        LTfloat scale = body->world->scale;
        // Second argument is array of polygon vertices.
        if (!lua_istable(L, 2)) {
            return luaL_error(L, "Expecting an array in second argument");
        }
        int len = lua_objlen(L, 2);
        if (len & 1) {
            return luaL_error(L, "Expecting an even number of elements in vertex array");
        }
        int num_vertices = len / 2;
        if (num_vertices < 3) {
            return luaL_error(L, "Need at least 3 vertices");
        }
        if (num_vertices > b2_maxPolygonVertices) {
            return luaL_error(L, "Too many vertices (at most %d allowed)", b2_maxPolygonVertices);
        }
        b2PolygonShape poly;
        b2Vec2 vertices[b2_maxPolygonVertices];
        for (int i = 0; i < num_vertices; i++) {
            lua_rawgeti(L, 2, i * 2 + 1);
            lua_rawgeti(L, 2, i * 2 + 2);
            vertices[i].x = luaL_checknumber(L, -2) / scale;
            vertices[i].y = luaL_checknumber(L, -1) / scale;
            lua_pop(L, 2);
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
        fixture_def.density = 1.0f;
        if (nargs >= 3) {
            read_fixture_attributes(L, 3, &fixture_def);
        }
        fixture_def.shape = &poly;
        LTFixture *fixture = new (lt_alloc_LTFixture(L)) LTFixture(body, &fixture_def);
        fixture->body_ref = ltLuaAddRef(L, 1, -1); // Add reference from body to new fixture.
        ltLuaAddNamedRef(L, -1, 1, "body"); // Add reference from fixture to body.
    } else {
        return luaL_error(L, "Cannot add fixtures to destroyed body");
    }
    return 1;
}

static int new_circle_fixture(lua_State *L) {
    int nargs = ltLuaCheckNArgs(L, 4);
    LTBody *body = lt_expect_LTBody(L, 1);
    if (body->body != NULL) {
        LTfloat scale = body->world->scale;
        LTfloat radius = luaL_checknumber(L, 2) / scale;
        LTfloat x = luaL_checknumber(L, 3) / scale;
        LTfloat y = luaL_checknumber(L, 4) / scale;
        b2CircleShape circle;
        circle.m_radius = radius;
        circle.m_p.Set(x, y);

        b2FixtureDef fixture_def;
        fixture_def.density = 1.0f;
        if (nargs >= 5) {
            read_fixture_attributes(L, 5, &fixture_def);
        }
        fixture_def.shape = &circle;
        LTFixture *fixture = new (lt_alloc_LTFixture(L)) LTFixture(body, &fixture_def);
        fixture->body_ref = ltLuaAddRef(L, 1, -1); // Add reference from body to new fixture.
        ltLuaAddNamedRef(L, -1, 1, "body"); // Add reference from fixture to body.
    } else {
        return luaL_error(L, "Cannot add fixtures to destroyed body");
    }
    return 1;
}

static int new_chain_fixture(lua_State *L) {
    int nargs = ltLuaCheckNArgs(L, 2);
    LTBody *body = lt_expect_LTBody(L, 1);
    if (body->body != NULL) {
        LTfloat scale = body->world->scale;
        // Second argument is array of chain vertices.
        if (!lua_istable(L, 2)) {
            return luaL_error(L, "Expecting an array in second argument");
        }
        int len = lua_objlen(L, 2);
        if (len < 4) {
            return luaL_error(L, "Chain requires at least 2 vertices");
        }
        if (len & 1) {
            return luaL_error(L, "Vertex list should have even length");
        }
        int num_vertices = len / 2;
        b2Vec2 *vertices = new b2Vec2[num_vertices];
        for (int i = 0; i < num_vertices; i++) {
            lua_rawgeti(L, 2, i * 2 + 1);
            lua_rawgeti(L, 2, i * 2 + 2);
            vertices[i].x = luaL_checknumber(L, -2) / scale;
            vertices[i].y = luaL_checknumber(L, -1) / scale;
            lua_pop(L, 2);
        }
        b2ChainShape chain;
        chain.CreateChain(vertices, num_vertices);
        delete[] vertices;

        // Third argument is a table of fixture properties.
        b2FixtureDef fixture_def;
        fixture_def.density = 1.0f;
        if (nargs >= 3) {
            read_fixture_attributes(L, 3, &fixture_def);
        }
        fixture_def.shape = &chain;
        LTFixture *fixture = new (lt_alloc_LTFixture(L)) LTFixture(body, &fixture_def);
        fixture->body_ref = ltLuaAddRef(L, 1, -1); // Add reference from body to new fixture.
        ltLuaAddNamedRef(L, -1, 1, "body"); // Add reference from fixture to body.
    } else {
        return luaL_error(L, "Cannot add fixtures to destroyed body");
    }
    return 1;
}

static LTObject* get_fixture_body(LTObject* obj) {
    LTFixture *f = (LTFixture*)obj;
    return f->body;
}

static int destroy_fixture(lua_State *L) {
    ltLuaCheckNArgs(L, 1); 
    LTFixture *fixture = lt_expect_LTFixture(L, 1);
    if (fixture->fixture != NULL) {
        fixture->destroy();
        ltLuaGetNamedRef(L, 1, "body"); // push body
        ltLuaDelRef(L, -1, fixture->body_ref); // Remove ref from body to fixture.
        lua_pop(L, 1);
        fixture->body_ref = LUA_NOREF;
    }
    return 0;
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

static int world_ray_cast(lua_State *L) {
    ltLuaCheckNArgs(L, 5);
    LTWorld *world = lt_expect_LTWorld(L, 1);
    LTfloat scale = world->scale;
    LTfloat x1 = luaL_checknumber(L, 2) / scale;
    LTfloat y1 = luaL_checknumber(L, 3) / scale;
    LTfloat x2 = luaL_checknumber(L, 4) / scale;
    LTfloat y2 = luaL_checknumber(L, 5) / scale;
    
    RayCastCallback cb;
    world->world->RayCast(&cb, b2Vec2(x1, y1), b2Vec2(x2, y2));

    lua_newtable(L);
    int i = 1;
    std::map<LTfloat, RayCastData>::iterator it;
    for (it = cb.hits.begin(); it != cb.hits.end(); it++) {
        lua_newtable(L);
    
        LTFixture *fixture = (LTFixture*)it->second.fixture->GetUserData();
        if (fixture->body != NULL) {
            ltLuaGetRef(L, 1, world->body_refs[fixture->body]); // push body
            ltLuaGetRef(L, -1, fixture->body_ref); // push fixture
            lua_setfield(L, -3, "fixture");
            lua_pop(L, 1); // pop body
        }

        lua_pushnumber(L, it->second.point.x * scale);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, it->second.point.y * scale);
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

struct AABBQueryCallBack : b2QueryCallback {
    lua_State *L;
    LTWorld *world;
    int i;

    AABBQueryCallBack(lua_State *L, LTWorld *world) {
        AABBQueryCallBack::L = L;
        AABBQueryCallBack::world = world;
        i = 1;
    }

    virtual bool ReportFixture(b2Fixture *fixture) {
        LTFixture *f = (LTFixture*)fixture->GetUserData();
        if (f->body != NULL) {
            ltLuaGetRef(L, 1, world->body_refs[f->body]); // push body
            ltLuaGetRef(L, -1, f->body_ref); // push fixture
            lua_rawseti(L, -3, i);
            lua_pop(L, 1); // pop body
            i++;
        }
        return true;
    }
};

static int world_find_fixtures_in(lua_State *L) {
    ltLuaCheckNArgs(L, 5);
    LTWorld *world = lt_expect_LTWorld(L, 1);
    LTfloat scale = world->scale;
    LTfloat x1 = (LTfloat)luaL_checknumber(L, 2) / scale;
    LTfloat y1 = (LTfloat)luaL_checknumber(L, 3) / scale;
    LTfloat x2 = (LTfloat)luaL_checknumber(L, 4) / scale;
    LTfloat y2 = (LTfloat)luaL_checknumber(L, 5) / scale;
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
    AABBQueryCallBack cb(L, world);
    lua_newtable(L);
    world->world->QueryAABB(&cb, aabb);
    return 1;
}

struct FixtureQueryCallBack : b2QueryCallback {
    lua_State *L;
    LTWorld *world;
    LTFixture *master;
    int i;

    FixtureQueryCallBack(lua_State *L, LTFixture *master) {
        FixtureQueryCallBack::L = L;
        FixtureQueryCallBack::master = master;
        world = master->body->world;
        i = 1;
    }

    virtual bool ReportFixture(b2Fixture *f) {
        LTFixture *fixture = (LTFixture*)f->GetUserData();
        if (fixture->body != NULL && fixture != master) {
            bool is_overlap = false;
            b2Shape *shape1 = fixture->fixture->GetShape();
            b2Shape *shape2 = master->fixture->GetShape();
            b2Transform transform1 = fixture->body->body->GetTransform();
            b2Transform transform2 = master->body->body->GetTransform();
            if (shape1->m_type == b2Shape::e_chain && shape2->m_type == b2Shape::e_chain) {
                // At least one shape must have volume for overlap.
                is_overlap = false;
            } else if (shape1->m_type == b2Shape::e_chain) {
                int n = shape1->GetChildCount();
                for (int c = 0; c < n; c++) {
                    is_overlap = b2TestOverlap(shape1, c, shape2, 0, transform1, transform2);
                    if (is_overlap) break;
                }
            } else if (shape2->m_type == b2Shape::e_chain) {
                int n = shape2->GetChildCount();
                for (int c = 0; c < n; c++) {
                    is_overlap = b2TestOverlap(shape1, 0, shape2, c, transform1, transform2);
                    if (is_overlap) break;
                }
            } else {
                is_overlap = b2TestOverlap(shape1, 0, shape2, 0, transform1, transform2);
            }
            if (is_overlap) {
                ltLuaGetNamedRef(L, 1, "body"); // push master body
                ltLuaGetNamedRef(L, -1, "world"); // push world
                ltLuaGetRef(L, -1, world->body_refs[fixture->body]); // push fixture body
                ltLuaGetRef(L, -1, fixture->body_ref); // push fixture
                lua_rawseti(L, -5, i);
                lua_pop(L, 3); // pop body, world, body
                i++;
            }
        }
        return true;
    }
};

static int fixture_find_overlaps(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTFixture *fixture = lt_expect_LTFixture(L, 1);
    lua_newtable(L);
    if (fixture->fixture != NULL) {
        b2Shape *shape = fixture->fixture->GetShape();
        b2AABB aabb;
        shape->ComputeAABB(&aabb, fixture->body->body->GetTransform(), 0);
        FixtureQueryCallBack cb(L, fixture);
        fixture->body->world->world->QueryAABB(&cb, aabb);
    }
    return 1;
}

LT_REGISTER_TYPE(LTWorld, "box2d.World", "lt.Object");
LT_REGISTER_PROPERTY_FLOAT(LTWorld, gx, get_world_gx, set_world_gx);
LT_REGISTER_PROPERTY_FLOAT(LTWorld, gy, get_world_gy, set_world_gy);
LT_REGISTER_PROPERTY_BOOL(LTWorld, auto_clear_forces, get_world_auto_clear_forces, set_world_auto_clear_forces);
LT_REGISTER_FIELD_FLOAT(LTWorld, scale);
LT_REGISTER_FIELD_BOOL(LTWorld, debug);
LT_REGISTER_METHOD(LTWorld, Step, world_step);
LT_REGISTER_METHOD(LTWorld, Body, new_body);
LT_REGISTER_METHOD(LTWorld, RayCast, world_ray_cast);
LT_REGISTER_METHOD(LTWorld, FixturesIn, world_find_fixtures_in);

LT_REGISTER_TYPE(LTBody, "box2d.Body", "lt.Wrap");
LT_REGISTER_PROPERTY_OBJ(LTBody, world, LTWorld, get_body_world, NULL);
LT_REGISTER_PROPERTY_FLOAT(LTBody, x, get_body_x, set_body_x);
LT_REGISTER_PROPERTY_FLOAT(LTBody, y, get_body_y, set_body_y);
LT_REGISTER_PROPERTY_FLOAT(LTBody, angle, get_body_angle, set_body_angle);
LT_REGISTER_PROPERTY_FLOAT(LTBody, vx, get_body_vx, set_body_vx);
LT_REGISTER_PROPERTY_FLOAT(LTBody, vy, get_body_vy, set_body_vy);
LT_REGISTER_PROPERTY_FLOAT(LTBody, angular_velocity, get_body_angular_velocity, set_body_angular_velocity);
LT_REGISTER_PROPERTY_BOOL(LTBody, destroyed, get_body_destroyed, NULL);
LT_REGISTER_METHOD(LTBody, Force, body_apply_force);
LT_REGISTER_METHOD(LTBody, Torque, body_apply_torque);
LT_REGISTER_METHOD(LTBody, Impulse, body_apply_impulse);
LT_REGISTER_METHOD(LTBody, AngularImpulse, body_apply_angular_impulse);
LT_REGISTER_METHOD(LTBody, Destroy, destroy_body);
LT_REGISTER_METHOD(LTBody, Polygon, new_polygon_fixture);
LT_REGISTER_METHOD(LTBody, Circle, new_circle_fixture);
LT_REGISTER_METHOD(LTBody, Chain, new_chain_fixture);

LT_REGISTER_TYPE(LTFixture, "box2d.Fixture", "lt.SceneNode");
LT_REGISTER_PROPERTY_OBJ(LTFixture, body, LTBody, get_fixture_body, NULL);
LT_REGISTER_METHOD(LTFixture, Destroy, destroy_fixture);
LT_REGISTER_METHOD(LTFixture, Touching, fixture_find_overlaps);
