/* Copyright (C) 2010 Ian MacLarty */

#include "ltphysics.h"

LTWorld::LTWorld(b2Vec2 gravity, bool doSleep) : LTObject(LT_TYPE_WORLD) {
    world = new b2World(gravity, doSleep);
}

LTWorld::~LTWorld() {
    b2Body *b = world->GetBodyList();
    // Release all body wrappers.
    while (b != NULL) {
        LTBody *ud = (LTBody*)b->GetUserData();
        ud->world = NULL;
        ud->body = NULL;
        // Don't call ud->release(), because that might cause this
        // LTWorld to be freed, resulting in infinite recursion to
        // ~LTWorld().
        ud->ref_count--;
        if (ud->ref_count <= 0) {
            delete ud;
        }
        b = b->GetNext();
    }
    delete world;
}

LTBody::LTBody(LTWorld *world, const b2BodyDef *def) : LTProp(LT_TYPE_BODY) {
    LTBody::world = world;
    body = world->world->CreateBody(def);
    body->SetUserData(this);
    LTObject::retain(); // Reference to this in the body user data.
}

// We record external references to bodies in the world too
// so that the world is not collected if there are no external
// references directly to it, but there are external references
// to bodies in the world.
void LTBody::retain() {
    if (world != NULL) {
        world->retain();
    }
    LTObject::retain();
}
void LTBody::release() {
    if (world != NULL) {
        world->release();
    }
    LTObject::release();
}

void LTBody::destroy() {
    if (body != NULL) { // NULL means the body was already destroyed.
        world->world->DestroyBody(body);
        body = NULL;
        LTObject::release(); // Release reference in body user data.
    }
}

void LTBody::draw() {
    if (body != NULL) {
        ltPushMatrix();
            b2Vec2 pos = body->GetPosition();
            ltTranslate(pos.x, pos.y, 0.0f);
            ltRotate(body->GetAngle() * LT_DEGREES_PER_RADIAN, 0.0f, 0.0f, 1.0f);
            b2Fixture *fixture = body->GetFixtureList();
            while (fixture != NULL) {
                b2Shape *shape = fixture->GetShape();
                switch (shape->m_type) {
                    case b2Shape::e_unknown:
                        break;
                    case b2Shape::e_circle: {
                        ltPushMatrix();
                            ltScale(shape->m_radius, shape->m_radius, 1.0f);
                            ltDrawUnitCircle();
                        ltPopMatrix();
                        break;
                    }
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
}
