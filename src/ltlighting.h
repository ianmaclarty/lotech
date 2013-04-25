/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltlighting)

// Enable/disable lighting
struct LTLightingNode : LTWrapNode {
    bool enabled;

    LTLightingNode();
    virtual void draw();
};

struct LTLight : LTWrapNode {
    LTColor ambient;
    LTColor diffuse;
    LTColor specular;
    LTVec3 position;
    LTfloat atten_c, atten_l, atten_q;
    LTbool fixed;

    LTLight();
    virtual void draw();
};

struct LTMaterial : LTWrapNode {
    LTfloat shininess;
    LTColor ambient;
    LTColor diffuse;
    LTColor specular;
    LTColor emission;

    LTMaterial();
    virtual void draw();

    void setup();
};
