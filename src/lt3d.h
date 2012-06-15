/* Copyright (C) 2011 Ian MacLarty */

LT_INIT_DECL(lt3d)

struct LTPerspective : LTWrapNode {
    LTfloat near;
    LTfloat origin;
    LTfloat far;
    LTfloat vanish_x;
    LTfloat vanish_y;

    LTPerspective() {near = 1; origin = 2; far = 10;};
    
    virtual void draw();
    bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

struct LTDepthTest : LTWrapNode {
    bool on;
    LTDepthTest() {on = true;}
    virtual void draw();
    bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

struct LTDepthMask : LTWrapNode {
    bool on;
    LTDepthMask() {on = true;}
    virtual void draw();
    bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

struct LTPitch : LTWrapNode {
    LTfloat pitch;

    virtual void draw();
    bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

struct LTFog : LTWrapNode {
    LTfloat start;
    LTfloat end;
    LTfloat red, green, blue;

    virtual void draw();
    bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};
