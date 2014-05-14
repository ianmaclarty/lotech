/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltscene)

struct LTSceneNode;

struct LTSceneNodeVisitor {
    virtual void visit(LTSceneNode *node) = 0;
};

struct LTSceneNode : LTObject {
    std::list<LTEventHandler *> *event_handlers;
    std::list<LTAction *> *actions;
    int active;
    LTfloat action_speed;

    LTSceneNode();
    virtual ~LTSceneNode();

    virtual void draw() {};
    virtual void visit_children(LTSceneNodeVisitor *v, bool reverse = false) {};

    // Returns false if inverse transform not possible or
    // not implemented.
    virtual bool inverse_transform(LTfloat *x, LTfloat *y) { return true; };

    void add_event_handler(LTEventHandler *handler);

    void enter(LTSceneNode *parent);
    void exit(LTSceneNode *parent);
    virtual void on_activate() {};
    virtual void on_deactivate() {};
    void add_action(LTAction *action);

    virtual bool containsPoint(LTfloat x, LTfloat y) { return false; }
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
    virtual void visit_children(LTSceneNodeVisitor *v, bool reverse);
};

struct LTWrapNode : LTSceneNode {
    LTSceneNode *child;

    LTWrapNode();
    
    virtual void init(lua_State *L);
    virtual void draw();
    virtual void visit_children(LTSceneNodeVisitor *v, bool reverse);
};

struct LTTranslateNode : LTWrapNode {
    LTfloat x;
    LTfloat y;
    LTfloat z;

    virtual void draw();
    virtual bool inverse_transform(LTfloat *x, LTfloat *y);
};

struct LTRotateNode : LTWrapNode {
    LTdegrees angle;
    // Center coords (about which to rotate).
    LTfloat cx;
    LTfloat cy;

    virtual void draw();
    virtual bool inverse_transform(LTfloat *x, LTfloat *y);
};

struct LTScaleNode : LTWrapNode {
    LTfloat scale_x;
    LTfloat scale_y;
    LTfloat scale_z;
    LTfloat scale;

    LTScaleNode() {scale_x = 1; scale_y = 1; scale_z = 1; scale = 1;};

    virtual void init(lua_State *L);
    virtual void draw();
    virtual bool inverse_transform(LTfloat *x, LTfloat *y);
};

struct LTShearNode : LTWrapNode {
    LTfloat xy;
    LTfloat xz;
    LTfloat yx;
    LTfloat yz;
    LTfloat zx;
    LTfloat zy;

    virtual void draw();
};

struct LTTransformNode : LTWrapNode {
    LTfloat m1;
    LTfloat m2;
    LTfloat m3;
    LTfloat m4;
    LTfloat m5;
    LTfloat m6;
    LTfloat m7;
    LTfloat m8;
    LTfloat m9;
    LTfloat m10;
    LTfloat m11;
    LTfloat m12;
    LTfloat m13;
    LTfloat m14;
    LTfloat m15;
    LTfloat m16;

    LTTransformNode();
    virtual void draw();
};

struct LTTintNode : LTWrapNode {
    LTfloat red;
    LTfloat green;
    LTfloat blue;
    LTfloat alpha;

    LTTintNode() {red = 1; green = 1; blue = 1; alpha = 1;};

    virtual void draw();
};

struct LTTextureModeNode : LTWrapNode {
    LTTextureMode mode;

    LTTextureModeNode() {};

    virtual void draw();
};

struct LTColorMaskNode : LTWrapNode {
    LTfloat red, green, blue, alpha;

    LTColorMaskNode() {red = 1; green = 1; blue = 1; alpha = 1;};

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

struct LTHiddenNode : LTWrapNode {
    virtual void draw();
};

LTSceneNode *lt_expect_LTSceneNode(lua_State *L, int arg);
LTLayer *lt_expect_LTLayer(lua_State *L, int arg);
LTWrapNode *lt_expect_LTWrapNode(lua_State *L, int arg);
bool lt_is_LTWrapNode(lua_State *L, int arg);
bool lt_is_LTSceneNode(lua_State *L, int arg);

void ltDeactivateAllScenes(lua_State *L);
