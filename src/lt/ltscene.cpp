/* Copyright (C) 2010 Ian MacLarty */

#include <math.h>

#include "ltimage.h"
#include "ltscene.h"

LTSceneNode::LTSceneNode(LTType type) : LTObject(type) {
    event_handlers = NULL;
}

LTSceneNode::~LTSceneNode() {
    if (event_handlers != NULL) {
        std::list<LTPointerEventHandler*>::iterator it;
        for (it = event_handlers->begin(); it != event_handlers->end(); it++) {
            delete *it;
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

LTWrapNode::LTWrapNode(LTSceneNode *child) : LTSceneNode(LT_TYPE_WRAP) {
    LTWrapNode::child = child;
}

LTWrapNode::LTWrapNode(LTSceneNode *child, LTType type) : LTSceneNode(type) {
    LTWrapNode::child = child;
}

void LTWrapNode::draw() {
    child->draw();
}

bool LTWrapNode::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (!consumePointerEvent(x, y, event)) {
        return child->propogatePointerEvent(x, y, event);
    } else {
        return true;
    }
}

LTfloat* LTWrapNode::field_ptr(const char *field_name) {
    return NULL;
}

LTLayer::LTLayer() : LTSceneNode(LT_TYPE_LAYER) {
}

#define NODEINDEX std::list<LTSceneNode*>::iterator

void LTLayer::insert_front(LTSceneNode *node) {
    node_list.push_back(node);
    node_index.insert(std::pair<LTSceneNode*, std::list<LTSceneNode*>::iterator>(node, --node_list.end()));
}

void LTLayer::insert_back(LTSceneNode *node) {
    node_list.push_front(node);
    node_index.insert(std::pair<LTSceneNode*, std::list<LTSceneNode*>::iterator>(node, node_list.begin()));
}

int LTLayer::size() {
    return node_list.size();
}

void LTLayer::remove(LTSceneNode *node) {
    std::pair<std::multimap<LTSceneNode*, std::list<LTSceneNode*>::iterator>::iterator,
              std::multimap<LTSceneNode*, std::list<LTSceneNode*>::iterator>::iterator> range;
    std::multimap<LTSceneNode*, std::list<LTSceneNode*>::iterator>::iterator it;
    range = node_index.equal_range(node);
    for (it = range.first; it != range.second; it++) {
        node_list.erase(it->second);
    }
    node_index.erase(range.first, range.second);
}

void LTLayer::clear() {
    node_list.clear();
    node_index.clear();
}

void LTLayer::draw() {
    int n = node_list.size();
    if (n == 0) {
        return;
    }
    if (n == 1) {
        (*node_list.begin())->draw();
        return;
    }
    std::list<LTSceneNode*>::iterator it;
    for (it = node_list.begin(); it != node_list.end(); it++) {
        ltPushMatrix();
        (*it)->draw();
        ltPopMatrix();
    }
}

bool LTLayer::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (!consumePointerEvent(x, y, event)) {
        std::list<LTSceneNode*>::reverse_iterator it;
        for (it = node_list.rbegin(); it != node_list.rend(); it++) {
            if ((*it)->propogatePointerEvent(x, y, event)) {
                return true;
            }
        }
        return false;
    } else {
        return true;
    }
}

LTTranslateNode::LTTranslateNode(LTfloat x, LTfloat y, LTfloat z, LTSceneNode *child)
    : LTWrapNode(child, LT_TYPE_TRANSLATE)
{
    LTTranslateNode::x = x;
    LTTranslateNode::y = y;
    LTTranslateNode::z = z;
}

void LTTranslateNode::draw() {
    ltTranslate(x, y, z);
    child->draw();
}

bool LTTranslateNode::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    LTfloat x1, y1;
    x1 = x - LTTranslateNode::x;
    y1 = y - LTTranslateNode::y;
    if (z != 0.0f || !consumePointerEvent(x1, y1, event)) {
        return child->propogatePointerEvent(x1, y1, event);
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
    return NULL;
}

LTRotateNode::LTRotateNode(LTdegrees angle, LTSceneNode *child) : LTWrapNode(child, LT_TYPE_ROTATE) {
    LTRotateNode::angle = angle;
}

void LTRotateNode::draw() {
    ltRotate(angle, 0.0f, 0.0f, 1.0f);
    child->draw();
}

bool LTRotateNode::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    LTfloat x1, y1;
    LTfloat a = -angle * LT_RADIANS_PER_DEGREE;
    LTfloat s = sinf(a);
    LTfloat c = cosf(a);
    x1 = c * x - s * y;
    y1 = s * x + c * y;
    if (!consumePointerEvent(x1, y1, event)) {
        return child->propogatePointerEvent(x1, y1, event);
    } else {
        return true;
    }
}

LTfloat* LTRotateNode::field_ptr(const char *field_name) {
    if (strcmp(field_name, "angle") == 0) {
        return &angle;
    }
    return NULL;
}

LTScaleNode::LTScaleNode(LTfloat sx, LTfloat sy, LTfloat s, LTSceneNode *child) : LTWrapNode(child, LT_TYPE_SCALE) {
    LTScaleNode::sx = sx;
    LTScaleNode::sy = sy;
    LTScaleNode::s = s;
}

void LTScaleNode::draw() {
    ltScale(sx * s, sy * s, 1.0f);
    child->draw();
}

bool LTScaleNode::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    LTfloat x1, y1;
    if (sx != 0.0f && sy != 0.0f && s != 0.0f) {
        x1 = x / (sx * s);
        y1 = y / (sy * s);
        if (!consumePointerEvent(x1, y1, event)) {
            return child->propogatePointerEvent(x1, y1, event);
        } else {
            return true;
        }
    } else {
        return false;
    }
}

LTfloat* LTScaleNode::field_ptr(const char *field_name) {
    if (strcmp(field_name, "scale") == 0) {
        return &s;
    }
    if (strcmp(field_name, "scale_x") == 0) {
        return &sx;
    }
    if (strcmp(field_name, "scale_y") == 0) {
        return &sy;
    }
    return NULL;
}

LTTintNode::LTTintNode(LTfloat r, LTfloat g, LTfloat b, LTfloat a, LTSceneNode *child) : LTWrapNode(child, LT_TYPE_TINT) {
    LTTintNode::r = r;
    LTTintNode::g = g;
    LTTintNode::b = b;
    LTTintNode::a = a;
}

void LTTintNode::draw() {
    if (a != 0.0f) {
        ltPushTint(r, g, b, a);
        child->draw();
        ltPopTint();
    }
}

bool LTTintNode::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (!consumePointerEvent(x, y, event)) {
        return child->propogatePointerEvent(x, y, event);
    } else {
        return true;
    }
}

LTfloat* LTTintNode::field_ptr(const char *field_name) {
    if (strcmp(field_name, "red") == 0) {
        return &r;
    }
    if (strcmp(field_name, "green") == 0) {
        return &g;
    }
    if (strcmp(field_name, "blue") == 0) {
        return &b;
    }
    if (strcmp(field_name, "alpha") == 0) {
        return &a;
    }
    return NULL;
}

LTBlendModeNode::LTBlendModeNode(LTBlendMode mode, LTSceneNode *child) : LTWrapNode(child, LT_TYPE_TINT) {
    LTBlendModeNode::blend_mode = mode;
}

void LTBlendModeNode::draw() {
    ltPushBlendMode(blend_mode);
    child->draw();
    ltPopBlendMode();
}

LTfloat* LTBlendModeNode::field_ptr(const char *field_name) {
    return NULL;
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

LTHitFilter::LTHitFilter(LTfloat left, LTfloat bottom, LTfloat right, LTfloat top, LTSceneNode *child) : LTWrapNode(child, LT_TYPE_HITFILTER) {
    LTHitFilter::left = left;
    LTHitFilter::bottom = bottom;
    LTHitFilter::right = right;
    LTHitFilter::top = top;
}

void LTHitFilter::draw() {
    child->draw();
}

bool LTHitFilter::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (x >= left && x <= right && y >= bottom && y <= top) {
        if (!consumePointerEvent(x, y, event)) {
            return child->propogatePointerEvent(x, y, event);
        } else {
            return true;
        }
    } else {
        return false;
    }
}

LTfloat* LTHitFilter::field_ptr(const char *field_name) {
    if (strcmp(field_name, "left") == 0) {
        return &left;
    }
    if (strcmp(field_name, "bottom") == 0) {
        return &bottom;
    }
    if (strcmp(field_name, "right") == 0) {
        return &right;
    }
    if (strcmp(field_name, "top") == 0) {
        return &top;
    }
    return NULL;
}
