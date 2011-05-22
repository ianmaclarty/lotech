/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTSCENE_H
#define LTSCENE_H

#include <list>
#include <map>

#include "ltevent.h"
#include "ltgraphics.h"

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
    // XXX deprecated in favour of LTHitFilter.
    virtual bool containsPoint(LTfloat x, LTfloat y) { return false; }

    // The scene node takes over ownership of the handler and
    // will free it when the scene node is freed.
    void addHandler(LTPointerEventHandler *handler);
};

// Layers group nodes together.
struct LTLayer : LTSceneNode {
    std::list<LTSceneNode*> node_list; // Nodes in draw order.
    std::multimap<LTSceneNode*, std::list<LTSceneNode*>::iterator> node_index;

    LTLayer();

    void insert_front(LTSceneNode *node); // node drawn last
    void insert_back(LTSceneNode *node);  // node drawn first
    void remove(LTSceneNode *node);
    void clear();
    int size();

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

struct LTWrapNode : LTSceneNode {
    LTSceneNode *child;

    LTWrapNode(LTSceneNode *child);
    LTWrapNode(LTSceneNode *child, LTType type);
    
    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTTranslateNode : LTWrapNode {
    LTfloat x;
    LTfloat y;
    LTfloat z;

    LTTranslateNode(LTfloat x, LTfloat y, LTfloat z, LTSceneNode *child);

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTRotateNode : LTWrapNode {
    LTdegrees angle;

    LTRotateNode(LTdegrees angle, LTSceneNode *child);

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTScaleNode : LTWrapNode {
    LTfloat sx;
    LTfloat sy;
    LTfloat s;

    LTScaleNode(LTfloat sx, LTfloat sy, LTfloat s, LTSceneNode *child);

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTTintNode : LTWrapNode {
    LTfloat r;
    LTfloat g;
    LTfloat b;
    LTfloat a;

    LTTintNode(LTfloat r, LTfloat g, LTfloat b, LTfloat a, LTSceneNode *child);

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTBlendModeNode : LTWrapNode {
    LTBlendMode blend_mode;

    LTBlendModeNode(LTBlendMode mode, LTSceneNode *child);

    virtual void draw();

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

struct LTHitFilter : LTWrapNode {
    LTfloat left, bottom, right, top;

    LTHitFilter(LTfloat left, LTfloat bottom, LTfloat right, LTfloat top, LTSceneNode *child);
    
    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
    virtual LTfloat* field_ptr(const char *field_name);
};

#endif
