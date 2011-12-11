#include "lt3d.h"
#include "ltgraphics.h"
#include "ltimage.h"

LTPerspective::LTPerspective(LTfloat near, LTfloat origin, LTfloat far, bool depth_buf_on,
        LTSceneNode *child) : LTWrapNode(child, LT_TYPE_PERSPECTIVE)
{
    LTPerspective::near = near;
    LTPerspective::origin = origin;
    LTPerspective::far = far;
    LTPerspective::depth_buffer_on = depth_buf_on;
}

void LTPerspective::draw() {
    ltPushPerspective(near, origin, far);
    #ifdef LTDEPTHBUF
    if (depth_buffer_on) {
        glEnable(GL_DEPTH_TEST);
    }
    #endif
    child->draw();
    #ifdef LTDEPTHBUF
    if (depth_buffer_on) {
        glDisable(GL_DEPTH_TEST);
    }
    #endif
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
    }
    return NULL;
}

LTPitch::LTPitch(LTfloat pitch, LTSceneNode *child) : LTWrapNode(child, LT_TYPE_PITCH) {
    LTPitch::pitch = pitch;
}

void LTPitch::draw() {
    glRotatef(pitch, 1, 0, 0);
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
    glEnable(GL_FOG);
    glFogfv(GL_FOG_COLOR, (const GLfloat*)&color);
    glFogf(GL_FOG_START, start);
    glFogf(GL_FOG_END, end);
    child->draw();
    glDisable(GL_FOG);
}

bool LTFog::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    return false;
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
    } else if (strcmp(field_name, "alpha") == 0) {
        return &color.a;
    }
    return NULL;
}
