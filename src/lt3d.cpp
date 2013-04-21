/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(lt3d)

static bool depth_test_on = false;
static bool depth_mask_on = true;
static LTCullMode cull_mode = LT_CULL_OFF;

void LTPerspective::draw() {
    if (child != NULL) {
        ltPushPerspective(nearz, origin, farz, vanish_x, vanish_y);
        child->draw();
        ltPopPerspective();
    }
}

bool LTPerspective::inverse_transform(LTfloat *x, LTfloat *y) {
    return false;
}

LT_REGISTER_TYPE(LTPerspective, "lt.Perspective", "lt.Wrap")
LT_REGISTER_FIELD_FLOAT_AS(LTPerspective, nearz, "near");
LT_REGISTER_FIELD_FLOAT(LTPerspective, origin);
LT_REGISTER_FIELD_FLOAT_AS(LTPerspective, farz, "far");
LT_REGISTER_FIELD_FLOAT(LTPerspective, vanish_x);
LT_REGISTER_FIELD_FLOAT(LTPerspective, vanish_y);

void LTCullFace::draw() {
    if (child != NULL) {
        LTCullMode prev_mode = cull_mode;
        cull_mode = mode;
        ltCullFace(mode);
        child->draw();
        ltCullFace(prev_mode);
        cull_mode = prev_mode;
    }
}

static const LTEnumConstant CullMode_enum_vals[] = {
    {"back",    LT_CULL_BACK},
    {"front",   LT_CULL_FRONT},
    {"off",     LT_CULL_OFF},
    {NULL, 0}};
LT_REGISTER_TYPE(LTCullFace, "lt.CullFace", "lt.Wrap")
LT_REGISTER_FIELD_ENUM(LTCullFace, mode, LTCullMode, CullMode_enum_vals);

void LTDepthTest::draw() {
    if (child != NULL) {
        bool prev_depth_test = depth_test_on;
        if (on) {
            ltEnableDepthTest();
            depth_test_on = true;
        } else {
            ltDisableDepthTest();
            depth_test_on = false;
        }
        child->draw();
        if (prev_depth_test != depth_test_on) {
            if (prev_depth_test) {
                ltEnableDepthTest();
            } else {
                ltDisableDepthTest();
            }
            depth_test_on = prev_depth_test;
        }
    }
}

LT_REGISTER_TYPE(LTDepthTest, "lt.DepthTest", "lt.Wrap")
LT_REGISTER_FIELD_BOOL(LTDepthTest, on)

void LTDepthMask::draw() {
    if (child != NULL) {
        bool prev_depth_mask = depth_mask_on;
        if (on) {
            ltEnableDepthMask();
            depth_mask_on = true;
        } else{
            ltDisableDepthMask();
            depth_mask_on = false;
        }
        child->draw();
        if (prev_depth_mask != depth_mask_on) {
            if (prev_depth_mask) {
                ltEnableDepthMask();
            } else {
                ltDisableDepthMask();
            }
            depth_mask_on = prev_depth_mask;
        }
    }
}

LT_REGISTER_TYPE(LTDepthMask, "lt.DepthMask", "lt.Wrap")
LT_REGISTER_FIELD_BOOL(LTDepthMask, on)

void LTPitch::draw() {
    if (child != NULL) {
        ltRotate(pitch, 1, 0, 0);
        child->draw();
    }
}

bool LTPitch::inverse_transform(LTfloat *x, LTfloat *y) {
    return false;
}

LT_REGISTER_TYPE(LTPitch, "lt.Pitch", "lt.Wrap")
LT_REGISTER_FIELD_FLOAT(LTPitch, pitch)

void LTFog::draw() {
    if (child != NULL) {
        ltEnableFog();
        ltFogColor(red, green, blue);
        ltFogStart(start);
        ltFogEnd(end);
        child->draw();
        ltDisableFog();
    }
}

LT_REGISTER_TYPE(LTFog, "lt.Fog", "lt.Wrap")
LT_REGISTER_FIELD_FLOAT(LTFog, start)
LT_REGISTER_FIELD_FLOAT(LTFog, end)
LT_REGISTER_FIELD_FLOAT(LTFog, red)
LT_REGISTER_FIELD_FLOAT(LTFog, green)
LT_REGISTER_FIELD_FLOAT(LTFog, blue)
