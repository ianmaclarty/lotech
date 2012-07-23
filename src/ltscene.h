/* Copyright (C) 2010 Ian MacLarty */
LT_INIT_DECL(ltscene)

struct LTSceneNode;

struct LTSceneNodeVisitor {
    virtual void visit(LTSceneNode *node) = 0;
};

struct LTSceneNode : LTObject {
    std::list<LTPointerEventHandler *> *event_handlers;
    std::list<LTAction *> *actions;
    int active;

    LTSceneNode();
    virtual ~LTSceneNode();

    virtual void draw() {};
    virtual void visitChildren(LTSceneNodeVisitor *v) {};

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

    void enter(LTSceneNode *parent);
    void exit(LTSceneNode *parent);
    virtual void on_activate() {};
    virtual void on_deactivate() {};

    void add_action(LTAction *action);
    // Called before changing OpenGL context.
    //virtual void preContextChange() {};
    // Called after changing OpenGL context.
    //virtual void postContextChange() {};
};

struct LTLayerNodeRefPair {
    LTSceneNode *node;
    int ref;
    LTLayerNodeRefPair(LTSceneNode *n, int r) {
        node = n;
        ref = r;
    }
};

// Layers group nodes together.
struct LTLayer : LTSceneNode {
    std::list<LTLayerNodeRefPair> node_list; // Nodes in draw order.
    std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator> node_index;

    void insert_front(LTSceneNode *node, int ref); // node drawn last
    void insert_back(LTSceneNode *node, int ref);  // node drawn first

    // Returns whether the existing node was found.
    // If the existing not was not found the new one is not inserted.
    bool insert_above(LTSceneNode *existing_node, LTSceneNode *new_node, int ref);
    bool insert_below(LTSceneNode *existing_node, LTSceneNode *new_node, int ref);

    void remove(lua_State *L, int layer_index, LTSceneNode *node);
    int size();

    virtual void draw();
    virtual void visitChildren(LTSceneNodeVisitor *v);
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

struct LTWrapNode : LTSceneNode {
    LTSceneNode *child;

    LTWrapNode();
    
    virtual void init(lua_State *L);
    virtual void draw();
    virtual void visitChildren(LTSceneNodeVisitor *v);
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

struct LTTranslateNode : LTWrapNode {
    LTfloat x;
    LTfloat y;
    LTfloat z;

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

struct LTRotateNode : LTWrapNode {
    LTdegrees angle;
    // Center coords (about which to rotate).
    LTfloat cx;
    LTfloat cy;

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

struct LTScaleNode : LTWrapNode {
    LTfloat scale_x;
    LTfloat scale_y;
    LTfloat scale_z;
    LTfloat scale;

    LTScaleNode() {scale_x = 1; scale_y = 1; scale_z = 1; scale = 1;};

    virtual void init(lua_State *L);
    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

struct LTTintNode : LTWrapNode {
    LTfloat red;
    LTfloat green;
    LTfloat blue;
    LTfloat alpha;

    LTTintNode() {red = 1; green = 1; blue = 1; alpha = 1;};

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

struct LTTextureModeNode : LTWrapNode {
    LTTextureMode mode;

    LTTextureModeNode() {};

    virtual void draw();
};

struct LTBlendModeNode : LTWrapNode {
    LTBlendMode mode;

    LTBlendModeNode() {};

    virtual void draw();
};

struct LTRectNode : LTSceneNode {
    LTfloat x1, y1, x2, y2;

    LTRectNode() {};
    
    virtual void draw();
};

// Filters all pointer events.
struct LTHitFilter : LTWrapNode {
    LTfloat left, bottom, right, top;

    LTHitFilter() {};
    
    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

// Filters only down events.
struct LTDownFilter : LTWrapNode {
    LTfloat left, bottom, right, top;

    LTDownFilter() {};
    
    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

LTSceneNode *lt_expect_LTSceneNode(lua_State *L, int arg);
LTLayer *lt_expect_LTLayer(lua_State *L, int arg);
LTWrapNode *lt_expect_LTWrapNode(lua_State *L, int arg);
bool lt_is_LTWrapNode(lua_State *L, int arg);
bool lt_is_LTSceneNode(lua_State *L, int arg);

void ltDeactivateAllScenes(lua_State *L);
