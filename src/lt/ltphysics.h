/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTPHYSICS_H
#define LTPHYSICS_H

#include "Box2D/Box2D.h"

#include "ltcommon.h"

struct LTWorld : LTObject {
    b2World *world;

    LTWorld(b2Vec2 gravity, bool doSleep);
    virtual ~LTWorld();
};

struct LTBody : LTObject {
    b2Body *body; // May be null if the body is destroyed.

    // Userdata is always a pointer to the LTBody object, so
    // don't pass it in the body def.
    LTBody(LTWorld *world, const b2BodyDef def);
};

#endif
