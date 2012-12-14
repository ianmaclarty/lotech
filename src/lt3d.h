/* Copyright (C) 2011 Ian MacLarty */

LT_INIT_DECL(lt3d)

struct LTPerspective : LTWrapNode {
    LTfloat nearz;
    LTfloat origin;
    LTfloat farz;
    LTfloat vanish_x;
    LTfloat vanish_y;

    LTPerspective() {nearz = 1; origin = 2; farz = 10;};
    
    virtual void draw();
    bool inverse_transform(LTfloat *x, LTfloat *y);
};

struct LTCullFace : LTWrapNode {
    LTCullMode mode;
    LTCullFace() { mode = LT_CULL_BACK; }
    virtual void draw();
};

struct LTDepthTest : LTWrapNode {
    bool on;
    LTDepthTest() {on = true;}
    virtual void draw();
};

struct LTDepthMask : LTWrapNode {
    bool on;
    LTDepthMask() {on = true;}
    virtual void draw();
};

struct LTPitch : LTWrapNode {
    LTfloat pitch;

    virtual void draw();
    bool inverse_transform(LTfloat *x, LTfloat *y);
};

struct LTFog : LTWrapNode {
    LTfloat start;
    LTfloat end;
    LTfloat red, green, blue;

    virtual void draw();
};
