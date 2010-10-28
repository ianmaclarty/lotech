/* Copyright (C) 2010 Ian MacLarty */
#include "ltgraphics.h"
#include "ltscene.h"

/* Buffered state */
static LTfloat current_red      = 1.0f;
static LTfloat current_green    = 1.0f;
static LTfloat current_blue     = 1.0f;
static LTfloat current_alpha    = 1.0f;

inline static void setCurrentColor() {
    glColor4f(current_red, current_green, current_blue, current_alpha);
}

LTSceneNode::LTSceneNode() {
    status = LT_STATUS_VISIBLE;

    x       = 0.0f;
    y       = 0.0f;
    angle   = 0.0f;
    x_scale = 1.0f;
    y_scale = 1.0f;

    red     = 1.0f;
    green   = 1.0f;
    blue    = 1.0f;
    alpha   = 1.0f;
}

LTfloat* LTSceneNode::getFloatField(LTTweenFloatField f) {
    switch (f) {
        case LT_FIELD_X: return &x;
        case LT_FIELD_Y: return &y;
        case LT_FIELD_ALPHA: return &alpha;
        case LT_FIELD_RED: return &red;
        case LT_FIELD_GREEN: return &green;
        case LT_FIELD_BLUE: return &blue;
        case LT_FIELD_X_SCALE: return &x_scale;
        case LT_FIELD_Y_SCALE: return &y_scale;
        case LT_FIELD_ANGLE: return &angle;
        default: return NULL;
    }
}

void ltRenderScene(LTSceneNode *scene) {
    LTfloat red = current_red;
    LTfloat green = current_green;
    LTfloat blue = current_blue;
    LTfloat alpha = current_alpha;
    std::list<LTSceneNode*>::iterator child;

    // Set up modelview matrix and color.
    glPushMatrix();
    if (scene->x != 0.0f || scene->y != 0.0f) {
        glTranslatef(scene->x, scene->y, 0.0f);
    }
    if (scene->angle != 0.0f) {
        glRotatef(scene->angle, 0.0f, 0.0f, 1.0f);
    }
    if (scene->x_scale != 1.0f || scene->y_scale != 1.0f) {
        glScalef(scene->x_scale, scene->y_scale, 1.0f);
    }
    if (scene->red != 1.0f) {
        current_red *= scene->red;
    }
    if (scene->green != 1.0f) {
        current_green *= scene->green;
    }
    if (scene->blue != 1.0f) {
        current_blue *= scene->blue;
    }
    if (scene->alpha != 1.0f) {
        current_alpha *= scene->alpha;
    }

    scene->preRenderChildren();

    // Render children.
    for (child = scene->children.begin(); child != scene->children.end(); child++) {
        ltRenderScene(*child);
    }

    scene->postRenderChildren();

    // Restore modelview matrix and color.
    glPopMatrix();
    current_red = red;
    current_green = green;
    current_blue = blue;
    current_alpha = alpha;
}

void LTUnitSquare::preRenderChildren() {
    static bool initialized = false;
    static const GLfloat vertices[] = {-0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f};
    static GLuint buffer_id;

    if (!initialized) {
        glGenBuffers(1, &buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, vertices, GL_STATIC_DRAW);
        initialized = true;
    }

    setCurrentColor();
    ltDisableTextures();
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    glVertexPointer(2, GL_FLOAT, 0, 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
