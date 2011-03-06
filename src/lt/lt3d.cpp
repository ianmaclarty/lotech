#include "lt3d.h"
#include "ltgraphics.h"
#include "ltimage.h"

LTPerspective::LTPerspective(LTfloat near, LTfloat origin, LTfloat far, LTSceneNode *child) : LTSceneNode(LT_TYPE_PERSPECTIVE) {
    LTPerspective::near = near;
    LTPerspective::origin = origin;
    LTPerspective::far = far;
    LTPerspective::child = child;
}

void LTPerspective::draw() {
    ltPushPerspective(near, origin, far);
    #ifdef LTDEPTHBUF
    glEnable(GL_DEPTH_TEST);
    #endif
    child->draw();
    #ifdef LTDEPTHBUF
    glDisable(GL_DEPTH_TEST);
    #endif
    ltPopPerspective();
}

LTfloat* LTPerspective::field_ptr(const char *field_name) {
    if (strcmp(field_name, "near") == 0) {
        return &near;
    } else if (strcmp(field_name, "origin") == 0) {
        return &origin;
    } else if (strcmp(field_name, "far") == 0) {
        return &far;
    } else {
        return NULL;
    }
}

LTPitch::LTPitch(LTfloat pitch, LTSceneNode *child) : LTSceneNode(LT_TYPE_PITCH) {
    LTPitch::pitch = pitch;
    LTPitch::child = child;
}

void LTPitch::draw() {
    ltPushMatrix();
    glRotatef(pitch, 1, 0, 0);
    child->draw();
    ltPopMatrix();
}

LTfloat* LTPitch::field_ptr(const char *field_name) {
    if (strcmp(field_name, "pitch") == 0) {
        return &pitch;
    }
    return NULL;
}

LTCuboidNode::LTCuboidNode(LTfloat width, LTfloat height, LTfloat depth) : LTSceneNode(LT_TYPE_CUBOID) {
    GLfloat w2 = width * 0.5f;
    GLfloat h2 = height * 0.5f;
    GLfloat d2 = depth * 0.5f;

    GLfloat vertices[] = { 
        -w2,  h2,  d2,
         w2,  h2,  d2,
        -w2, -h2,  d2,
         w2, -h2,  d2,
        -w2,  h2, -d2,
         w2,  h2, -d2,
         w2, -h2, -d2,
        -w2, -h2, -d2,
    };

    GLubyte front_elems[] = {0, 1, 3, 2};
    GLubyte back_elems[] = {4, 5, 6, 7};
    GLubyte top_elems[] = {4, 5, 1, 0};
    GLubyte bottom_elems[] = {7, 6, 3, 2};
    GLubyte left_elems[] = {0, 4, 7, 2};
    GLubyte right_elems[] = {1, 5, 6, 3};

    glGenBuffers(1, &vertbuf);
    glGenBuffers(1, &frontbuf);
    glGenBuffers(1, &backbuf);
    glGenBuffers(1, &topbuf);
    glGenBuffers(1, &bottombuf);
    glGenBuffers(1, &leftbuf);
    glGenBuffers(1, &rightbuf);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertbuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, frontbuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(front_elems), front_elems, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backbuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(back_elems), back_elems, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, topbuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(top_elems), top_elems, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bottombuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bottom_elems), bottom_elems, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, leftbuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(left_elems), left_elems, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rightbuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(right_elems), right_elems, GL_STATIC_DRAW);
}

LTCuboidNode::~LTCuboidNode() {
    glDeleteBuffers(1, &vertbuf);
    glDeleteBuffers(1, &frontbuf);
    glDeleteBuffers(1, &backbuf);
    glDeleteBuffers(1, &topbuf);
    glDeleteBuffers(1, &bottombuf);
    glDeleteBuffers(1, &leftbuf);
    glDeleteBuffers(1, &rightbuf);
}

void LTCuboidNode::draw() {
    ltDisableTextures();
    glBindBuffer(GL_ARRAY_BUFFER, vertbuf);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    ltPushTint(0.4f, 0.4f, 0.4f, 1.0f);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backbuf);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0);
    ltPopTint();

    ltPushTint(0.7f, 0.7f, 0.7f, 1.0f);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, leftbuf);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0);
    ltPopTint();

    ltPushTint(0.8f, 0.8f, 0.8f, 1.0f);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rightbuf);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0);
    ltPopTint();

    ltPushTint(0.5f, 0.5f, 0.5f, 1.0f);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, topbuf);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0);
    ltPopTint();

    ltPushTint(0.6f, 0.6f, 0.6f, 1.0f);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bottombuf);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0);
    ltPopTint();

    ltPushTint(0.9f, 0.9f, 0.9f, 1.0f);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, frontbuf);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, 0);
    ltPopTint();
}
