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
        ud->body = NULL;
        ud->release();
        b = b->GetNext();
    }
    delete world;
}

LTBody::LTBody(LTWorld *world, const b2BodyDef *def) : LTObject(LT_TYPE_BODY) {
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
    world->retain();
    LTObject::retain();
}
void LTBody::release() {
    world->release();
    LTObject::release();
}

void LTBody::destroy() {
    world->world->DestroyBody(body);
    body = NULL;
    LTObject::release(); // Release reference in body user data.
}
