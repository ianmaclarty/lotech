/* Copyright (C) 2010 Ian MacLarty */

#include <assert.h>

#include "ltphysics.h"
#include "ltgraphics.h"

LTWorld::LTWorld(b2Vec2 gravity, bool doSleep) : LTObject(LT_TYPE_WORLD) {
    world = new b2World(gravity, doSleep);
}

LTWorld::~LTWorld() {
    b2Body *b = world->GetBodyList();
    // Free all body and fixture wrappers.
    // They must all have a reference count of 1, otherwise the
    // LTWorld wouldn't be deleted.
    while (b != NULL) {
        LTBody *bdy = (LTBody*)b->GetUserData();
        b2Fixture *f = b->GetFixtureList();
        while (f != NULL) {
            LTFixture *fix = (LTFixture*)f->GetUserData();
            assert(fix->ref_count == 1);
            delete fix;
            f = f->GetNext();
        }
        assert(bdy->ref_count == 1);
        delete bdy;
        b = b->GetNext();
    }
    delete world;
}

LTBody::LTBody(LTWorld *world, const b2BodyDef *def) : LTSceneNode(LT_TYPE_BODY) {
    LTBody::world = world;
    body = world->world->CreateBody(def);
    body->SetUserData(this);
    ltRetain(this); // Reference to this in the body user data.
                    // This causes the LTBody not to be collected until
                    // the b2Body is destroyed.
}

// We record external references to bodies in the world too
// so that the world is not collected if there are no external
// references directly to it, but there are external references
// to bodies in the world.
void LTBody::retain() {
    world->retain();
    ltRetain(this);
}
void LTBody::release() {
    ltRelease(this);
    world->release();
}

void LTBody::destroy() {
    if (body != NULL) { // NULL means the body was already destroyed.

        // Release fixture wrappers.
        b2Fixture *f = body->GetFixtureList();
        while (f != NULL) {
            LTFixture *ud = (LTFixture*)f->GetUserData();
            ud->fixture = NULL;
            // Release the reference to the fixture which was added when we created it.
            // We don't call ud->release(), because that would release the body too.
            ltRelease(ud);
            f = f->GetNext();
        }

        world->world->DestroyBody(body);
        body = NULL;
        ltRelease(this); // Release reference in body user data.
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
                LTFixture *f = (LTFixture*)fixture->GetUserData();
                f->draw();
                fixture = fixture->GetNext();
            }
        ltPopMatrix();
    }
}

bool LTBody::containsPoint(LTfloat x, LTfloat y) {
    b2Vec2 p = b2Vec2(x, y);
    b2Fixture *fixture = body->GetFixtureList();
    while (fixture != NULL) {
        if (fixture->TestPoint(p)) {
            return true;
        }
        fixture = fixture->GetNext();
    }
    return false;
}

LTFixture::LTFixture(LTBody *body, const b2FixtureDef *def) : LTSceneNode(LT_TYPE_FIXTURE) {
    LTFixture::body = body;
    if (body->body != NULL) {
        fixture = body->body->CreateFixture(def);
        fixture->SetUserData(this);
        ltRetain(this); // Reference to this in the fixture user data.
                        // This causes the LTFixture not to be collected until
                        // the b2Fixture is destroyed.
    } else {
        fixture = NULL;
    }
}

// We record external references to fixtures in the body too
// so that the body is not collected if there are no external
// references directly to it, but there are external references
// to fixtures in the body.
void LTFixture::retain() {
    body->retain();
    ltRetain(this);
}
void LTFixture::release() {
    ltRelease(this);
    body->release();
}

void LTFixture::destroy() {
    if (fixture != NULL) { // NULL means the fixture was already destroyed.
        body->body->DestroyFixture(fixture);
        fixture = NULL;
        ltRelease(this); // Release reference in fixture user data.
    }
}

void LTFixture::draw() {
    if (fixture != NULL) {
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
