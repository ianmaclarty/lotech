#include "lt.h"

LT_INIT_IMPL(lt3d)

static bool depth_test_on = false;
static bool depth_mask_on = true;

void LTPerspective::draw() {
    ltPushPerspective(near, origin, far, vanish_x, vanish_y);
    child->draw();
    ltPopPerspective();
}

bool LTPerspective::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    return false;
}

LT_REGISTER_TYPE(LTPerspective, "lt.Perspective", "lt.Wrap")
LT_REGISTER_FIELD_FLOAT(LTPerspective, near);
LT_REGISTER_FIELD_FLOAT(LTPerspective, origin);
LT_REGISTER_FIELD_FLOAT(LTPerspective, far);
LT_REGISTER_FIELD_FLOAT(LTPerspective, vanish_x);
LT_REGISTER_FIELD_FLOAT(LTPerspective, vanish_y);

void LTDepthTest::draw() {
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

bool LTDepthTest::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    return child->propogatePointerEvent(x, y, event);
}

LT_REGISTER_TYPE(LTDepthTest, "lt.DepthTest", "lt.Wrap")
LT_REGISTER_FIELD_BOOL(LTDepthTest, on)

void LTDepthMask::draw() {
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

bool LTDepthMask::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    return child->propogatePointerEvent(x, y, event);
}

LT_REGISTER_TYPE(LTDepthMask, "lt.DepthMask", "lt.Wrap")
LT_REGISTER_FIELD_BOOL(LTDepthMask, on)

void LTPitch::draw() {
    ltRotate(pitch, 1, 0, 0);
    child->draw();
}

bool LTPitch::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    return false;
}

LT_REGISTER_TYPE(LTPitch, "lt.Pitch", "lt.Wrap")
LT_REGISTER_FIELD_FLOAT(LTPitch, pitch)

void LTFog::draw() {
    ltEnableFog();
    ltFogColor(red, green, blue);
    ltFogStart(start);
    ltFogEnd(end);
    child->draw();
    ltDisableFog();
}

bool LTFog::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    return child->propogatePointerEvent(x, y, event);
}

LT_REGISTER_TYPE(LTFog, "lt.Fog", "lt.Wrap")
LT_REGISTER_FIELD_FLOAT(LTFog, start)
LT_REGISTER_FIELD_FLOAT(LTFog, end)
LT_REGISTER_FIELD_FLOAT(LTFog, red)
LT_REGISTER_FIELD_FLOAT(LTFog, green)
LT_REGISTER_FIELD_FLOAT(LTFog, blue)
