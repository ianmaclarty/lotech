#include "lt.h"

static bool depth_test_on = false;
static bool depth_mask_on = true;

LTPerspective::LTPerspective(LTfloat near, LTfloat origin, LTfloat far,
        LTfloat vanish_x, LTfloat vanish_y, LTSceneNode *child) : LTWrapNode(child, LT_TYPE_PERSPECTIVE)
{
    LTPerspective::near = near;
    LTPerspective::origin = origin;
    LTPerspective::far = far;
    LTPerspective::vanish_x = vanish_x;
    LTPerspective::vanish_y = vanish_y;
}

void LTPerspective::draw() {
    ltPushPerspective(near, origin, far, vanish_x, vanish_y);
    child->draw();
    ltPopPerspective();
}

bool LTPerspective::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    return false;
}

LTFieldDescriptor* LTPerspective::fields() {
    static LTFieldDescriptor flds[] = {
        {"near", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(near), NULL, NULL, LT_ACCESS_FULL},
        {"origin", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(origin), NULL, NULL, LT_ACCESS_FULL},
        {"far", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(far), NULL, NULL, LT_ACCESS_FULL},
        {"vanish_x", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(vanish_x), NULL, NULL, LT_ACCESS_FULL},
        {"vanish_y", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(vanish_y), NULL, NULL, LT_ACCESS_FULL},
        LT_END_FIELD_DESCRIPTOR_LIST
    };
    return flds;
}

LTDepthTest::LTDepthTest(bool on, LTSceneNode *child) : LTWrapNode(child, LT_TYPE_DEPTHTEST)
{
    LTDepthTest::on = on;
}

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

LTDepthMask::LTDepthMask(bool on, LTSceneNode *child) : LTWrapNode(child, LT_TYPE_DEPTHMASK)
{
    LTDepthMask::on = on;
}

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

LTPitch::LTPitch(LTfloat pitch, LTSceneNode *child) : LTWrapNode(child, LT_TYPE_PITCH) {
    LTPitch::pitch = pitch;
}

void LTPitch::draw() {
    ltRotate(pitch, 1, 0, 0);
    child->draw();
}

bool LTPitch::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    return false;
}

LTFieldDescriptor* LTPitch::fields() {
    static LTFieldDescriptor flds[] = {
        {"pitch", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(pitch), NULL, NULL, LT_ACCESS_FULL},
        LT_END_FIELD_DESCRIPTOR_LIST
    };
    return flds;
}

LTFog::LTFog(LTfloat start, LTfloat end, LTColor color, LTSceneNode *child)
    : LTWrapNode(child, LT_TYPE_FOG)
{
    LTFog::start = start;
    LTFog::end = end;
    LTFog::color = color;
}

void LTFog::draw() {
    ltEnableFog();
    ltFogColor(color.r, color.g, color.b);
    ltFogStart(start);
    ltFogEnd(end);
    child->draw();
    ltDisableFog();
}

bool LTFog::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    return child->propogatePointerEvent(x, y, event);
}

LTFieldDescriptor* LTFog::fields() {
    static LTFieldDescriptor flds[] = {
        {"start", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(start), NULL, NULL, LT_ACCESS_FULL},
        {"end", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(end), NULL, NULL, LT_ACCESS_FULL},
        {"red", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(color.r), NULL, NULL, LT_ACCESS_FULL},
        {"green", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(color.g), NULL, NULL, LT_ACCESS_FULL},
        {"blue", LT_FIELD_TYPE_FLOAT, LT_OFFSETOF(color.b), NULL, NULL, LT_ACCESS_FULL},
        LT_END_FIELD_DESCRIPTOR_LIST
    };
    return flds;
}
