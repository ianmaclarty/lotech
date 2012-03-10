#include "lt3d.h"
#include "ltgraphics.h"
#include "ltimage.h"

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

LTfloat* LTPerspective::field_ptr(const char *field_name) {
    if (strcmp(field_name, "near") == 0) {
        return &near;
    } else if (strcmp(field_name, "origin") == 0) {
        return &origin;
    } else if (strcmp(field_name, "far") == 0) {
        return &far;
    } else if (strcmp(field_name, "vanish_x") == 0) {
        return &vanish_x;
    } else if (strcmp(field_name, "vanish_y") == 0) {
        return &vanish_y;
    }
    return NULL;
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

LTfloat* LTPitch::field_ptr(const char *field_name) {
    if (strcmp(field_name, "pitch") == 0) {
        return &pitch;
    }
    return NULL;
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

LTfloat* LTFog::field_ptr(const char *field_name) {
    if (strcmp(field_name, "start") == 0) {
        return &start;
    } else if (strcmp(field_name, "end") == 0) {
        return &end;
    } else if (strcmp(field_name, "red") == 0) {
        return &color.r;
    } else if (strcmp(field_name, "green") == 0) {
        return &color.g;
    } else if (strcmp(field_name, "blue") == 0) {
        return &color.b;
    }
    return NULL;
}
