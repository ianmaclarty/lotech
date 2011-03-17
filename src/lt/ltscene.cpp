/* Copyright (C) 2010 Ian MacLarty */

#include <math.h>

#include "ltgraphics.h"
#include "ltimage.h"
#include "ltscene.h"

LTSceneNode::LTSceneNode(LTType type) : LTObject(type) {
    event_handlers = NULL;
}

LTSceneNode::~LTSceneNode() {
    if (event_handlers != NULL) {
        std::list<LTPointerEventHandler*>::iterator it;
        for (it = event_handlers->begin(); it != event_handlers->end(); it++) {
            delete (*it);
        }
        delete event_handlers;
    }
}

bool LTSceneNode::consumePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    bool consumed = false;
    if (event_handlers != NULL) {
        std::list<LTPointerEventHandler*>::iterator it;
        for (it = event_handlers->begin(); it != event_handlers->end(); it++) {
            if ((*it)->consume(x, y, this, event)) {
                consumed = true;
            }
        }
    }
    return consumed;
}

bool LTSceneNode::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    return consumePointerEvent(x, y, event);
}

void LTSceneNode::addHandler(LTPointerEventHandler *handler) {
    if (event_handlers == NULL) {
        event_handlers = new std::list<LTPointerEventHandler *>();
    }
    event_handlers->push_front(handler);
}

LTLayer::LTLayer() : LTSceneNode(LT_TYPE_LAYER) {
}

#define NODEINDEX std::multimap<LTfloat, LTSceneNode*>::iterator

void LTLayer::insert(LTSceneNode *node, LTfloat depth) {
    std::pair<LTfloat, LTSceneNode*> val(depth, node);
    node_index.insert(std::pair<LTSceneNode*, NODEINDEX>(node, layer.insert(val)));
}

void LTLayer::remove(LTSceneNode *node) {
    std::pair<std::multimap<LTSceneNode*, NODEINDEX>::iterator, std::multimap<LTSceneNode*, NODEINDEX>::iterator> range;
    std::multimap<LTSceneNode*, NODEINDEX>::iterator it;
    range = node_index.equal_range(node);
    for (it = range.first; it != range.second; it++) {
        layer.erase((*it).second);
    }
    node_index.erase(range.first, range.second);
}

void LTLayer::draw() {
    std::multimap<LTfloat, LTSceneNode*>::iterator it;
    for (it = layer.begin(); it != layer.end(); it++) {
        ((*it).second)->draw();
    }
}

bool LTLayer::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (!consumePointerEvent(x, y, event)) {
        std::multimap<LTfloat, LTSceneNode*>::reverse_iterator it;
        for (it = layer.rbegin(); it != layer.rend(); it++) {
            if (((*it).second)->propogatePointerEvent(x, y, event)) {
                return true;
            }
        }
        return false;
    } else {
        return true;
    }
}

LTTranslateNode::LTTranslateNode(LTfloat x, LTfloat y, LTfloat z, LTSceneNode *child) : LTSceneNode(LT_TYPE_TRANSLATE) {
    LTTranslateNode::x = x;
    LTTranslateNode::y = y;
    LTTranslateNode::z = z;
    LTTranslateNode::child = child;
}

void LTTranslateNode::draw() {
    ltPushMatrix();
    ltTranslate(x, y, z);
    child->draw();
    ltPopMatrix();
}

bool LTTranslateNode::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (z != 0.0f || !consumePointerEvent(x, y, event)) {
        return child->propogatePointerEvent(x - LTTranslateNode::x, y - LTTranslateNode::y, event);
    } else {
        return true;
    }
}

LTfloat* LTTranslateNode::field_ptr(const char *field_name) {
    if (strcmp(field_name, "x") == 0) {
        return &x;
    }
    if (strcmp(field_name, "y") == 0) {
        return &y;
    }
    if (strcmp(field_name, "z") == 0) {
        return &z;
    }
    return child->field_ptr(field_name);
}

LTRotateNode::LTRotateNode(LTdegrees angle, LTSceneNode *child) : LTSceneNode(LT_TYPE_ROTATE) {
    LTRotateNode::angle = angle;
    LTRotateNode::child = child;
}

void LTRotateNode::draw() {
    ltPushMatrix();
    ltRotate(angle, 0.0f, 0.0f, 1.0f);
    child->draw();
    ltPopMatrix();
}

bool LTRotateNode::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (!consumePointerEvent(x, y, event)) {
        LTfloat a = -angle * LT_RADIANS_PER_DEGREE;
        LTfloat s = sinf(a);
        LTfloat c = cosf(a);
        return child->propogatePointerEvent(c * x - s * y, s * x + c * y, event);
    } else {
        return true;
    }
}

LTfloat* LTRotateNode::field_ptr(const char *field_name) {
    if (strcmp(field_name, "angle") == 0) {
        return &angle;
    }
    return child->field_ptr(field_name);
}

LTScaleNode::LTScaleNode(LTfloat sx, LTfloat sy, LTSceneNode *child) : LTSceneNode(LT_TYPE_SCALE) {
    LTScaleNode::sx = sx;
    LTScaleNode::sy = sy;
    LTScaleNode::child = child;
}

void LTScaleNode::draw() {
    ltPushMatrix();
    ltScale(sx, sy, 1.0f);
    child->draw();
    ltPopMatrix();
}

bool LTScaleNode::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (!consumePointerEvent(x, y, event)) {
        if (sx != 0.0f && sy != 0.0f) {
            return child->propogatePointerEvent(x / sx, y / sy, event);
        } else {
            return false;
        }
    } else {
        return true;
    }
}

LTfloat* LTScaleNode::field_ptr(const char *field_name) {
    if (strcmp(field_name, "sx") == 0) {
        return &sx;
    }
    if (strcmp(field_name, "sy") == 0) {
        return &sy;
    }
    return child->field_ptr(field_name);
}

LTTintNode::LTTintNode(LTfloat r, LTfloat g, LTfloat b, LTfloat a, LTSceneNode *child) : LTSceneNode(LT_TYPE_TINT) {
    LTTintNode::r = r;
    LTTintNode::g = g;
    LTTintNode::b = b;
    LTTintNode::a = a;
    LTTintNode::child = child;
}

void LTTintNode::draw() {
    ltPushTint(r, g, b, a);
    child->draw();
    ltPopTint();
}

bool LTTintNode::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (!consumePointerEvent(x, y, event)) {
        return child->propogatePointerEvent(x, y, event);
    } else {
        return true;
    }
}

LTfloat* LTTintNode::field_ptr(const char *field_name) {
    if (strcmp(field_name, "r") == 0) {
        return &r;
    }
    if (strcmp(field_name, "g") == 0) {
        return &g;
    }
    if (strcmp(field_name, "b") == 0) {
        return &b;
    }
    if (strcmp(field_name, "a") == 0) {
        return &a;
    }
    return child->field_ptr(field_name);
}

LTLineNode::LTLineNode(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2) : LTSceneNode(LT_TYPE_LINE) {
    LTLineNode::x1 = x1;
    LTLineNode::y1 = y1;
    LTLineNode::x2 = x2;
    LTLineNode::y2 = y2;
}

void LTLineNode::draw() {
    ltDisableTextures();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexPointer(2, GL_FLOAT, 0, &x1);
    glDrawArrays(GL_LINE_STRIP, 0, 2);
}

LTfloat* LTLineNode::field_ptr(const char *field_name) {
    if (strcmp(field_name, "x1") == 0) {
        return &x1;
    }
    if (strcmp(field_name, "y1") == 0) {
        return &y1;
    }
    if (strcmp(field_name, "x2") == 0) {
        return &x2;
    }
    if (strcmp(field_name, "y2") == 0) {
        return &y2;
    }
    return NULL;
}

LTTriangleNode::LTTriangleNode(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2, LTfloat x3, LTfloat y3) : LTSceneNode(LT_TYPE_TRIANGLE) {
    LTTriangleNode::x1 = x1;
    LTTriangleNode::y1 = y1;
    LTTriangleNode::x2 = x2;
    LTTriangleNode::y2 = y2;
    LTTriangleNode::x3 = x3;
    LTTriangleNode::y3 = y3;
}

void LTTriangleNode::draw() {
    ltDisableTextures();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexPointer(2, GL_FLOAT, 0, &x1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
}

LTfloat* LTTriangleNode::field_ptr(const char *field_name) {
    if (strcmp(field_name, "x1") == 0) {
        return &x1;
    }
    if (strcmp(field_name, "y1") == 0) {
        return &y1;
    }
    if (strcmp(field_name, "x2") == 0) {
        return &x2;
    }
    if (strcmp(field_name, "y2") == 0) {
        return &y2;
    }
    if (strcmp(field_name, "x3") == 0) {
        return &x3;
    }
    if (strcmp(field_name, "y3") == 0) {
        return &y3;
    }
    return NULL;
}

LTRectNode::LTRectNode(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2) : LTSceneNode(LT_TYPE_RECT) {
    LTRectNode::x1 = x1;
    LTRectNode::y1 = y1;
    LTRectNode::x2 = x2;
    LTRectNode::y2 = y2;
}

void LTRectNode::draw() {
    ltDisableTextures();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GLfloat v[] = {
        x1, y1,
        x2, y1,
        x2, y2,
        x1, y2
    };
    glVertexPointer(2, GL_FLOAT, 0, v);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

LTfloat* LTRectNode::field_ptr(const char *field_name) {
    if (strcmp(field_name, "x1") == 0) {
        return &x1;
    }
    if (strcmp(field_name, "y1") == 0) {
        return &y1;
    }
    if (strcmp(field_name, "x2") == 0) {
        return &x2;
    }
    if (strcmp(field_name, "y2") == 0) {
        return &y2;
    }
    return NULL;
}

