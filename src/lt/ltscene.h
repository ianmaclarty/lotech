/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTSCENE_H
#define LTSCENE_H

#include <list>
#include <map>

#include "ltevent.h"

struct LTSceneNode : LTObject {
    std::list<LTPointerEventHandler *> *event_handlers;

    LTSceneNode(LTType type);
    virtual ~LTSceneNode();

    virtual void draw() = 0;

    // Call all the event handles for this node only, returning true iff at least one
    // of the event handlers returns true.
    bool consumePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    // Propogate an event to this and all descendent nodes, calling event
    // handlers along the way.  If at least one of the event handlers for a particular
    // node returns true, then propogation stops.
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    // Returns true iff this node, or one of its descendents, contains the given point.
    virtual bool containsPoint(LTfloat x, LTfloat y) { return false; }

    void addHandler(LTPointerEventHandler *handler);
};

// Layers group nodes together.
struct LTLayer : LTSceneNode {
    std::multimap<LTfloat, LTSceneNode*> layer;
    std::multimap<LTSceneNode*, std::multimap<LTfloat, LTSceneNode*>::iterator> node_index;

    LTLayer();

    void insert(LTSceneNode *node, LTfloat depth);
    void remove(LTSceneNode *node);

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

struct LTTranslateNode : LTSceneNode {
    LTfloat x;
    LTfloat y;
    LTSceneNode *child;

    LTTranslateNode(LTfloat x, LTfloat y, LTSceneNode *child);

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTRotateNode : LTSceneNode {
    LTdegrees angle;
    LTSceneNode *child;

    LTRotateNode(LTdegrees angle, LTSceneNode *child);

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTScaleNode : LTSceneNode {
    LTfloat sx;
    LTfloat sy;
    LTSceneNode *child;

    LTScaleNode(LTfloat sx, LTfloat sy, LTSceneNode *child);

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTTintNode : LTSceneNode {
    LTfloat r;
    LTfloat g;
    LTfloat b;
    LTfloat a;
    LTSceneNode *child;

    LTTintNode(LTfloat r, LTfloat g, LTfloat b, LTfloat a, LTSceneNode *child);

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTLineNode : LTSceneNode {
    LTfloat x1, y1, x2, y2;

    LTLineNode(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2);
    
    virtual void draw();

    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTTriangleNode : LTSceneNode {
    LTfloat x1, y1, x2, y2, x3, y3;

    LTTriangleNode(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2, LTfloat x3, LTfloat y3);
    
    virtual void draw();

    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTRectNode : LTSceneNode {
    LTfloat x1, y1, x2, y2;

    LTRectNode(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2);
    
    virtual void draw();

    virtual LTfloat* field_ptr(const char *field_name);
};

#endif
