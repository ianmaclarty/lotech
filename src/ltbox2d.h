/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltbox2d)

struct LTBody;

struct LTWorld : LTObject {
    b2World *world;
    LTfloat scale; // For scaling world coords to screen coords.
    LTbool debug;
    std::map<LTBody*, int> body_refs;

    LTWorld();
    virtual ~LTWorld();
};

struct LTBody : LTWrapNode {
    b2Body *body; // May be null if the body is destroyed.
    LTWorld *world;
    int world_ref;

    LTBody() {
        ltAbort();
    };

    // Userdata is always a pointer to the LTBody object, so
    // leave it null in the body def.
    LTBody(LTWorld *world, const b2BodyDef *def);

    void destroy();

    virtual void draw();
    //virtual bool containsPoint(LTfloat x, LTfloat y);
    virtual bool inverse_transform(LTfloat *x, LTfloat *y);
};

struct LTFixture : LTSceneNode {
    b2Fixture *fixture; // May be null if the fixture is destroyed.
    LTBody *body;
    int body_ref;

    LTFixture() {
        ltAbort();
    };

    // Userdata is always a pointer to the LTFixture object, so
    // leave it null in the fixture def. 
    LTFixture(LTBody *body, const b2FixtureDef *def);

    void destroy();

    virtual void draw();
};

struct LTJoint : LTObject {
    b2Joint *joint; // May be null if the joint is destroyed.
    LTWorld *world;

    LTJoint() {
        ltAbort();
    };

    LTJoint(LTWorld *world, const b2JointDef *def);

    void destroy();
};

struct LTBodyTracker : LTWrapNode {
    LTBody *body;
    // Use viewport mode when the child is a layer containing
    // the body being tracked.  The layer will then follow
    // the body.
    bool viewport_mode;
    bool track_rotation;
    LTfloat min_x, max_x, min_y, max_y;
    LTfloat snap_to;  // If not zero, snap to nearest multiple of this value.

    LTBodyTracker(LTBody *body, LTSceneNode *child, bool viewport_mode, bool track_rotation,
        LTfloat min_x, LTfloat max_x, LTfloat min_y, LTfloat max_y, LTfloat snap_to);

    virtual bool inverse_transform(LTfloat *x, LTfloat *y);
    virtual void draw();
};

// Check if Box2D will allow the polygon to be attached to a body
// without an assertion failure.
bool ltCheckB2Poly(const b2Vec2* vs, int32 count);
