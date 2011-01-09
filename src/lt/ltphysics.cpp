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

LTBody::LTBody(LTWorld *world, const b2BodyDef def) : LTObject(LT_TYPE_BODY) {
    b2BodyDef def2 = def;
    def2.userData = this;
    body = world->world->CreateBody(&def2);
    retain(); // retained by world.
}
