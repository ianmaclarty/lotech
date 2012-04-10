/* Copyright (C) 2011 Ian MacLarty */

struct LTPerspective : LTWrapNode {
    LTfloat near;
    LTfloat origin;
    LTfloat far;
    LTfloat vanish_x;
    LTfloat vanish_y;

    LTPerspective(LTfloat near, LTfloat origin, LTfloat far, LTfloat vanish_x, LTfloat vanish_y,
        LTSceneNode *child);
    
    virtual void draw();
    bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
    virtual LTFieldDescriptor* fields();
};

struct LTDepthTest : LTWrapNode {
    bool on;
    LTDepthTest(bool on, LTSceneNode *child);
    virtual void draw();
    bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

struct LTDepthMask : LTWrapNode {
    bool on;
    LTDepthMask(bool on, LTSceneNode *child);
    virtual void draw();
    bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

struct LTPitch : LTWrapNode {
    LTfloat pitch;

    LTPitch(LTfloat pitch, LTSceneNode *child);

    virtual void draw();
    bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
    virtual LTFieldDescriptor* fields();
};

struct LTFog : LTWrapNode {
    LTfloat start;
    LTfloat end;
    LTColor color;

    LTFog(LTfloat start, LTfloat end, LTColor color, LTSceneNode *child);

    virtual void draw();
    bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
    virtual LTFieldDescriptor* fields();
};
