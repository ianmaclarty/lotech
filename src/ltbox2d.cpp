/* Copyright (C) 2010 Ian MacLarty */

#include "lt.h"

LT_INIT_IMPL(ltbox2d)

LTWorld::LTWorld() {
    world = new b2World(b2Vec2(0.0f, 0.0f));
    world->SetAllowSleeping(true);
    scaling = 1.0f;
}

LTWorld::~LTWorld() {
    delete world;
}

static LTfloat get_world_gx(LTObject *obj) {
    return ((LTWorld*)obj)->world->GetGravity().x;
}

static LTfloat get_world_gy(LTObject *obj) {
    return ((LTWorld*)obj)->world->GetGravity().y;
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

LT_REGISTER_TYPE(LTWorld, "box2d.World", "lt.Object");
LT_REGISTER_PROPERTY_FLOAT(LTWorld, gx, get_world_gx, set_world_gx);
LT_REGISTER_PROPERTY_FLOAT(LTWorld, gx, get_world_gy, set_world_gy);
LT_REGISTER_FIELD_FLOAT(LTWorld, scaling);

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

LT_REGISTER_METHOD(LTWorld, Step, world_step);

//LTBody::LTBody(LTWorld *world, const b2BodyDef *def) : LTSceneNode(LT_TYPE_BODY) {
//    LTBody::world = world;
//    body = world->world->CreateBody(def);
//    body->SetUserData(this);
//}
//
//void LTBody::destroy() {
//    if (body != NULL) { // NULL means the body was already destroyed.
//        // Invalidate fixture wrappers.
//        b2Fixture *f = body->GetFixtureList();
//        while (f != NULL) {
//            LTFixture *ud = (LTFixture*)f->GetUserData();
//            ud->fixture = NULL;
//            f = f->GetNext();
//        }
//        // Invalidate joints.
//        b2JointEdge *j = body->GetJointList();
//        while (j != NULL) {
//            LTJoint *ud = (LTJoint*)j->joint->GetUserData();
//            ud->joint = NULL;
//            j = j->next;
//        }
//
//        world->world->DestroyBody(body);
//        body = NULL;
//    }
//}
//
//void LTBody::draw() {
//    if (body != NULL) {
//        b2Vec2 pos = body->GetPosition();
//        ltTranslate(pos.x, pos.y, 0.0f);
//        ltRotate(body->GetAngle() * LT_DEGREES_PER_RADIAN, 0.0f, 0.0f, 1.0f);
//        b2Fixture *fixture = body->GetFixtureList();
//        while (fixture != NULL) {
//            LTFixture *f = (LTFixture*)fixture->GetUserData();
//            f->draw();
//            fixture = fixture->GetNext();
//        }
//    }
//}
//
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
//
//LTFixture::LTFixture(LTBody *body, const b2FixtureDef *def) : LTSceneNode(LT_TYPE_FIXTURE) {
//    LTFixture::body = body;
//    if (body->body != NULL) {
//        fixture = body->body->CreateFixture(def);
//        fixture->SetUserData(this);
//    } else {
//        fixture = NULL;
//    }
//}
//
//void LTFixture::destroy() {
//    if (fixture != NULL) { // NULL means the fixture was already destroyed.
//        body->body->DestroyFixture(fixture);
//        fixture = NULL;
//    }
//}
//
//void LTFixture::draw() {
//    if (fixture != NULL) {
//        b2Shape *shape = fixture->GetShape();
//        switch (shape->m_type) {
//            case b2Shape::e_edge:
//                break;
//            case b2Shape::e_chain:
//                break;
//            case b2Shape::e_circle: {
//                b2CircleShape *circle = (b2CircleShape *)shape;
//                ltPushMatrix();
//                ltPushTint(1.0f, 1.0f, 1.0f, 0.5f);
//                ltTranslate(circle->m_p.x, circle->m_p.y, 0.0f);
//                ltScale(circle->m_radius, circle->m_radius, 1.0f);
//                ltDrawUnitCircle();
//                ltPopTint();
//                ltPopMatrix();
//                break;
//            }
//            case b2Shape::e_polygon: {
//                b2PolygonShape *poly = (b2PolygonShape *)shape;
//                ltPushTint(1.0f, 1.0f, 1.0f, 0.5f);
//                ltDrawPoly((LTfloat *)poly->m_vertices, poly->m_vertexCount);
//                ltPopTint();
//                for (int i = 0; i < poly->m_vertexCount - 1; i++) {
//                    ltVertexPointer(2, LT_VERT_DATA_TYPE_FLOAT, 0, &poly->m_vertices[i]);
//                    ltDrawArrays(LT_DRAWMODE_LINE_STRIP, 0, 2);
//                }
//                b2Vec2 final_line[2];
//                final_line[0] = poly->m_vertices[poly->m_vertexCount - 1];
//                final_line[1] = poly->m_vertices[0];
//                ltVertexPointer(2, LT_VERT_DATA_TYPE_FLOAT, 0, final_line);
//                ltDrawArrays(LT_DRAWMODE_LINE_STRIP, 0, 2);
//                break;
//            }
//            case b2Shape::e_typeCount:
//                break;
//        }
//    }
//}
//
//LTJoint::LTJoint(LTWorld *world, const b2JointDef *def) : LTObject(LT_TYPE_JOINT) {
//    LTJoint::world = world;
//    LTJoint::joint = world->world->CreateJoint(def);
//    LTJoint::joint->SetUserData(this);
//}
//
//void LTJoint::destroy() {
//    if (joint != NULL) {
//        world->world->DestroyJoint(joint);
//        joint = NULL;
//    }
//}
//
//LTBodyTracker::LTBodyTracker(LTBody *body, LTSceneNode *child,
//    bool viewport_mode, bool track_rotation,
//    LTfloat min_x, LTfloat max_x, LTfloat min_y, LTfloat max_y, LTfloat snap_to)
//    : LTWrapNode(child, LT_TYPE_BODYTRACKER)
//{
//    LTBodyTracker::viewport_mode = viewport_mode;
//    LTBodyTracker::track_rotation = track_rotation;
//    LTBodyTracker::min_x = min_x;
//    LTBodyTracker::max_x = max_x;
//    LTBodyTracker::min_y = min_y;
//    LTBodyTracker::max_y = max_y;
//    LTBodyTracker::snap_to = snap_to;
//    LTWorld *w = body->world;
//    if (w != NULL) {
//        LTBodyTracker::scaling = w->scaling;
//    } else {
//        LTBodyTracker::scaling = 1.0f;
//    }
//    LTBodyTracker::body = body;
//}
//
//void LTBodyTracker::draw() {
//    static LTfloat rmat[16] = {
//        1.0f, 0.0f, 0.0f, 0.0f,
//        0.0f, 1.0f, 0.0f, 0.0f,
//        0.0f, 0.0f, 1.0f, 0.0f,
//        0.0f, 0.0f, 0.0f, 1.0f,
//    };
//    b2Body *b = body->body;
//    if (b != NULL) {
//        const b2Transform b2t = b->GetTransform();
//        LTfloat x = b2t.p.x * scaling;
//        LTfloat y = b2t.p.y * scaling;
//        if (snap_to > 0.0f) {
//            x = roundf(x / snap_to) * snap_to;
//            y = roundf(y / snap_to) * snap_to;
//        }
//        if (x < min_x) {
//            x = min_x;
//        } else if (x > max_x) {
//            x = max_x;
//        }
//        if (y < min_y) {
//            y = min_y;
//        } else if (y > max_y) {
//            y = max_y;
//        }
//        if (viewport_mode) {
//            if (track_rotation) {
//                rmat[0] = b2t.q.c;
//                rmat[1] = -b2t.q.s;
//                rmat[4] = b2t.q.s;
//                rmat[5] = b2t.q.c;
//                ltMultMatrix(rmat);
//            }
//            ltTranslate(-x, -y, 0.0f);
//        } else {
//            ltTranslate(x, y, 0.0f);
//            if (track_rotation) {
//                rmat[0] = b2t.q.c;
//                rmat[1] = b2t.q.s;
//                rmat[4] = -b2t.q.s;
//                rmat[5] = b2t.q.c;
//                ltMultMatrix(rmat);
//            }
//        }
//        child->draw();
//    }
//}
//
//bool LTBodyTracker::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
//    if (!consumePointerEvent(x, y, event)) {
//        b2Body *b = body->body;
//        if (b != NULL) {
//            // XXX This doesn't work with scaling or in viewport mode.
//            LTfloat angle = b->GetAngle();
//            b2Vec2 pos = b->GetPosition();
//            x = x - pos.x;
//            y = y - pos.y;
//            LTfloat x1, y1;
//            LTfloat s = sinf(angle);
//            LTfloat c = cosf(angle);
//            x1 = c * x - s * y;
//            y1 = s * x + c * y;
//            return child->propogatePointerEvent(x1, y1, event);
//        } else {
//            return false;
//        }
//    } else {
//        return true;
//    }
//}
//
//bool ltCheckB2Poly(const b2Vec2* vs, int32 count) {
//    // This code copied from Box2D (b2PolygonShape.cpp, ComputeCentroid).
//
//    if (count < 2) {
//        return false;
//    }
//    if (count == 2) {
//        return true;
//    }
//
//    b2Vec2 c;
//    c.Set(0.0f, 0.0f);
//    float32 area = 0.0f;
//    b2Vec2 pRef(0.0f, 0.0f);
//
//    const float32 inv3 = 1.0f / 3.0f;
//
//    for (int32 i = 0; i < count; ++i) {
//        b2Vec2 p1 = pRef;
//        b2Vec2 p2 = vs[i];
//        b2Vec2 p3 = i + 1 < count ? vs[i+1] : vs[0];
//
//        b2Vec2 e1 = p2 - p1;
//        b2Vec2 e2 = p3 - p1;
//
//        float32 D = b2Cross(e1, e2);
//
//        float32 triangleArea = 0.5f * D;
//        area += triangleArea;
//
//        c += triangleArea * inv3 * (p1 + p2 + p3);
//    }
//
//    return area > b2_epsilon;
//}
