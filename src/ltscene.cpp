/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltscene)

//static std::list<LTSceneNode *> all_nodes;
//static std::set<LTSceneNode *> roots;

//static void check_scene_nodes();

LTSceneNode::LTSceneNode() {
    event_handlers = NULL;
    active = 0;
    action_speed = 1.0f;
    //all_nodes.push_back(this);
}

LTSceneNode::~LTSceneNode() {
    assert(!active);
    if (event_handlers != NULL) {
        std::list<LTEventHandler*>::iterator it;
        for (it = event_handlers->begin(); it != event_handlers->end(); it++) {
            if (!(*it)->execution_pending) {
                delete *it;
            } else {
                (*it)->cancelled = true;
            }
        }
        delete event_handlers;
    }
    if (actions != NULL) {
        std::list<LTAction*>::iterator it;
        for (it = actions->begin(); it != actions->end(); it++) {
            LTAction *action = *it;
            assert(!action->scheduled);
            if (action->cancelled) {
                // This means it is in the cancelled list and so will
                // be deleted when that list is traversed
                // (see ltExecuteActions in ltaction.cpp).
                action->node = NULL;
            } else {
                delete *it;
            }
        }
        delete actions;
    }
    //all_nodes.remove(this);
}

void LTSceneNode::add_event_handler(LTEventHandler *handler) {
    if (event_handlers == NULL) {
        event_handlers = new std::list<LTEventHandler *>();
    }
    event_handlers->push_front(handler);
}

struct EnterVisitor : LTSceneNodeVisitor {
    int inc;
    EnterVisitor(int n) {
        inc = n;
    }
    virtual void visit(LTSceneNode *node) {
        if (!node->active) {
            node->on_activate();
            if (node->actions != NULL) {
                for (std::list<LTAction*>::iterator it = node->actions->begin(); it != node->actions->end(); it++) {
                    (*it)->schedule();
                }
            }
        }
        node->active += inc;
        node->visit_children(this);
    }
};

static void map_inc(std::map<LTSceneNode *, int> *ss, LTSceneNode *node) {
    if (ss->find(node) == ss->end()) {
        (*ss)[node] = 1;
    } else {
        (*ss)[node] = (*ss)[node] + 1;
    }
}

struct CheckVisitor : LTSceneNodeVisitor {
    std::map<LTSceneNode *, int> *ss;
    CheckVisitor(std::map<LTSceneNode *, int> *s) {
        ss = s;
    }
    virtual void visit(LTSceneNode *node) {
        map_inc(ss, node);
        CheckVisitor v2(ss);
        node->visit_children(&v2);
    }
};

void LTSceneNode::enter(LTSceneNode *parent) {
    if (parent == NULL || parent->active) {
        int n = parent == NULL ? 1 : parent->active;
        EnterVisitor v(n);
        v.visit(this);
    }
}

struct ExitVisitor : LTSceneNodeVisitor {
    int dec;
    ExitVisitor(int n) {
        dec = n;
    }
    virtual void visit(LTSceneNode *node) {
        node->visit_children(this);
        node->active -= dec;
        assert(node->active >= 0);
        if (node->active == 0) {
            if (node->actions != NULL) {
                for (std::list<LTAction*>::iterator it = node->actions->begin(); it != node->actions->end(); it++) {
                    (*it)->unschedule();
                }
            }
            node->on_deactivate();
            if (node == lt_exclusive_receiver) {
                lt_exclusive_receiver = NULL;
            }
        }
    }
};

void LTSceneNode::exit(LTSceneNode *parent) {
    if (parent == NULL || parent->active) {
        int n = parent == NULL ? 1 : parent->active;
        ExitVisitor v(n);
        v.visit(this);
    }
}

void LTSceneNode::add_action(LTAction *action) {
    if (actions == NULL) {
        actions = new std::list<LTAction*>();
    }
    if (action->node != this) {
        ltLog("LTSceneNode::add_action: invalid node");
        ltAbort();
    }
    if (action->no_dups) {
        std::list<LTAction*>::iterator it;
        for (it = actions->begin(); it != actions->end(); it++) {
            if ((*it)->action_id == action->action_id) {
                (*it)->cancel();
            }
        }
    }
    actions->push_back(action);
    if (active) {
        action->schedule();
    }
}

LT_REGISTER_TYPE(LTSceneNode, "lt.SceneNode", "lt.Object");

struct SpeedVisitor : LTSceneNodeVisitor {
    LTfloat spd;
    SpeedVisitor(LTfloat s) {
        spd = s;
    }
    virtual void visit(LTSceneNode *node) {
        node->action_speed = spd;
        node->visit_children(this);
    }
};

static void set_action_speed(LTObject *obj, LTfloat val) {
    LTSceneNode* node = (LTSceneNode*)obj;
    SpeedVisitor v(val);
    v.visit(node);
}

static LTfloat get_action_speed(LTObject *obj) {
    return ((LTSceneNode *)obj)->action_speed;
}

LT_REGISTER_PROPERTY_FLOAT_NOCONS(LTSceneNode, action_speed, &get_action_speed, &set_action_speed)

static void push_scene_roots_table(lua_State *L) {
    static void *scene_root_key;
    lua_pushlightuserdata(L, &scene_root_key);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (lua_isnil(L, -1)) {
        lua_pushlightuserdata(L, &scene_root_key);
        lua_newtable(L);
        lua_rawset(L, LUA_REGISTRYINDEX);
        lua_pushlightuserdata(L, &scene_root_key);
        lua_rawget(L, LUA_REGISTRYINDEX);
    }
}

static int activate_scene_node(lua_State *L) {
    //check_scene_nodes();
    ltLuaCheckNArgs(L, 1);
    LTSceneNode *node = lt_expect_LTSceneNode(L, 1);
    push_scene_roots_table(L);
    lua_pushvalue(L, 1);
    lua_rawget(L, -2);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1); // pop nil
        lua_pushvalue(L, 1);
        lua_pushboolean(L, 1);
        lua_rawset(L, -3);
        node->enter(NULL);
    } else {
        return luaL_error(L, "Scene node already active");
    }
    //roots.insert(node);
    //check_scene_nodes();
    return 0;
}

static int deactivate_scene_node(lua_State *L) {
    //check_scene_nodes();
    ltLuaCheckNArgs(L, 1);
    LTSceneNode *node = lt_expect_LTSceneNode(L, 1);
    push_scene_roots_table(L);
    lua_pushvalue(L, 1);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_pushvalue(L, 1);
        lua_pushnil(L);
        lua_rawset(L, -3);
        node->exit(NULL);
    } else {
        return luaL_error(L, "Scene node not active");
    }
    //assert(roots.find(node) != roots.end());
    //roots.erase(node);
    //check_scene_nodes();
    return 0;
}

LT_REGISTER_METHOD(LTSceneNode, Activate, activate_scene_node);
LT_REGISTER_METHOD(LTSceneNode, Deactivate, deactivate_scene_node);

static int pause_scene_node(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTSceneNode *node = lt_expect_LTSceneNode(L, 1);
    set_action_speed(node, 0.0f);
    return 0;
}
static int resume_scene_node(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTSceneNode *node = lt_expect_LTSceneNode(L, 1);
    set_action_speed(node, 1.0f);
    return 0;
}

LT_REGISTER_METHOD(LTSceneNode, Pause, pause_scene_node);
LT_REGISTER_METHOD(LTSceneNode, Resume, resume_scene_node);

void ltDeactivateAllScenes(lua_State *L) {
    push_scene_roots_table(L);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        LTSceneNode *root = lt_expect_LTSceneNode(L, -2);
        root->exit(NULL);
        lua_pop(L, 1); 
    }
    lua_pop(L, 1); // pop scene roots.
    //roots.clear();
}

LTWrapNode::LTWrapNode() {
    LTWrapNode::child = NULL;
}

void LTWrapNode::init(lua_State *L) {
    LTSceneNode::init(L);
}

void LTWrapNode::draw() {
    if (child != NULL) {
        child->draw();
    }
}

void LTWrapNode::visit_children(LTSceneNodeVisitor *v, bool reverse) {
    if (child != NULL) {
        v->visit(child);
    }
}

static LTObject *get_child(LTObject *obj) {
    return ((LTWrapNode*)obj)->child;
}

static void set_child(LTObject *obj, LTObject *val) {
    //check_scene_nodes();
    LTWrapNode *wrap = (LTWrapNode*)obj;
    LTSceneNode *old_child = wrap->child;
    LTSceneNode *new_child = (LTSceneNode*)val;
    if (old_child != NULL) {
        old_child->exit(wrap);
    }
    wrap->child = new_child;
    if (new_child != NULL) {
        new_child->enter(wrap);
    }
    //check_scene_nodes();
}

LT_REGISTER_TYPE(LTWrapNode, "lt.Wrap", "lt.SceneNode");
LT_REGISTER_PROPERTY_OBJ(LTWrapNode, child, LTSceneNode, get_child, set_child);

#define NODEINDEX std::list<LTSceneNode*>::iterator

void LTLayer::insert_front(LTSceneNode *node, int ref) {
    //check_scene_nodes();
    node_list.push_back(LTLayerNodeRefPair(node, ref));
    node_index.insert(std::pair<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>(node, --node_list.end()));
    node->enter(this);
    //check_scene_nodes();
}

void LTLayer::insert_back(LTSceneNode *node, int ref) {
    //check_scene_nodes();
    node_list.push_front(LTLayerNodeRefPair(node, ref));
    node_index.insert(std::pair<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>(node, node_list.begin()));
    node->enter(this);
    //check_scene_nodes();
}

bool LTLayer::insert_above(LTSceneNode *existing_node, LTSceneNode *new_node, int ref) {
    //check_scene_nodes();
    std::pair<std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>::iterator,
              std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>::iterator> range;
    std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>::iterator it;
    range = node_index.equal_range(existing_node);
    if (range.first != range.second) {
        std::list<LTLayerNodeRefPair>::iterator existing_it = (range.first)->second;
        std::list<LTLayerNodeRefPair>::iterator new_it = node_list.insert(++existing_it, LTLayerNodeRefPair(new_node, ref));
        node_index.insert(std::pair<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>(new_node, new_it));
        new_node->enter(this);
        //check_scene_nodes();
        return true;
    } else {
        return false;
    }
}

bool LTLayer::insert_below(LTSceneNode *existing_node, LTSceneNode *new_node, int ref) {
    //check_scene_nodes();
    std::pair<std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>::iterator,
              std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>::iterator> range;
    std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>::iterator it;
    range = node_index.equal_range(existing_node);
    if (range.first != range.second) {
        std::list<LTLayerNodeRefPair>::iterator existing_it = (range.first)->second;
        std::list<LTLayerNodeRefPair>::iterator new_it = node_list.insert(existing_it, LTLayerNodeRefPair(new_node, ref));
        node_index.insert(std::pair<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>(new_node, new_it));
        new_node->enter(this);
        //check_scene_nodes();
        return true;
    } else {
        return false;
    }
}

int LTLayer::size() {
    return node_list.size();
}

void LTLayer::remove(lua_State *L, int layer_index, LTSceneNode *node) {
    //check_scene_nodes();
    std::pair<std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>::iterator,
              std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>::iterator> range;
    std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>::iterator it;
    range = node_index.equal_range(node);
    for (it = range.first; it != range.second; it++) {
        ltLuaDelRef(L, layer_index, it->second->ref);
        node_list.erase(it->second);
        it->first->exit(this);
    }
    node_index.erase(range.first, range.second);
    //check_scene_nodes();
}

void LTLayer::draw() {
    int n = node_list.size();
    if (n == 0) {
        return;
    }
    if (n == 1) {
        (*node_list.begin()).node->draw();
        return;
    }
    std::list<LTLayerNodeRefPair>::iterator it;
    for (it = node_list.begin(); it != node_list.end(); it++) {
        ltPushMatrix();
        (*it).node->draw();
        ltPopMatrix();
    }
}

void LTLayer::visit_children(LTSceneNodeVisitor *v, bool reverse) {
    if (reverse) {
        std::list<LTLayerNodeRefPair>::reverse_iterator it;
        for (it = node_list.rbegin(); it != node_list.rend(); it++) {
            v->visit((*it).node);
        }
    } else {
        std::list<LTLayerNodeRefPair>::iterator it;
        for (it = node_list.begin(); it != node_list.end(); it++) {
            v->visit((*it).node);
        }
    }
}

LT_REGISTER_TYPE(LTLayer, "lt.Layer", "lt.SceneNode");

static int new_Layer(lua_State *L) {
    int num_args = ltLuaCheckNArgs(L, 0);
    LTLayer *layer = new (lt_alloc_LTLayer(L)) LTLayer();
    // Add arguments as child nodes.
    // First arguments are drawn in front of last arguments.
    for (int arg = 1; arg <= num_args; arg++) {
        LTSceneNode *child = lt_expect_LTSceneNode(L, arg);
        int ref = ltLuaAddRef(L, -1, arg); // Add reference from layer node to child node.
        layer->insert_back(child, ref);
    }
    return 1;
}

LT_REGISTER_METHOD(LTLayer, new, new_Layer);

void LTTranslateNode::draw() {
    if (child != NULL) {
        ltTranslate(x, y, z);
        child->draw();
    }
}

bool LTTranslateNode::inverse_transform(LTfloat *x1, LTfloat *y1) {
    *x1 -= x;
    *y1 -= y;
    return true;
}

LT_REGISTER_TYPE(LTTranslateNode, "lt.Translate", "lt.Wrap");
LT_REGISTER_FIELD_FLOAT(LTTranslateNode, x);
LT_REGISTER_FIELD_FLOAT(LTTranslateNode, y);
LT_REGISTER_FIELD_FLOAT(LTTranslateNode, z);

void LTRotateNode::draw() {
    if (child != NULL) {
        ltTranslate(cx, cy, 0.0f);
        ltRotate(angle, 0.0f, 0.0f, 1.0f);
        ltTranslate(-cx, -cy, 0.0f);
        child->draw();
    }
}

bool LTRotateNode::inverse_transform(LTfloat *x, LTfloat *y) {
    LTfloat a = -angle * LT_RADIANS_PER_DEGREE;
    LTfloat s = sinf(a);
    LTfloat c = cosf(a);
    LTfloat x1, y1;
    x1 = c * *x - s * *y;
    y1 = s * *x + c * *y;
    *x = x1;
    *y = y1;
    return true;
}

LT_REGISTER_TYPE(LTRotateNode, "lt.Rotate", "lt.Wrap");
LT_REGISTER_FIELD_FLOAT(LTRotateNode, angle);
LT_REGISTER_FIELD_FLOAT(LTRotateNode, cx);
LT_REGISTER_FIELD_FLOAT(LTRotateNode, cy);

void LTScaleNode::init(lua_State *L) {
    LTWrapNode::init(L);
    int nargs = lua_gettop(L) - 1;
    if (nargs == 2 && lua_isnumber(L, 2)) {
        // Only two arguments (child + number), so interpret second arg as the
        // scale value, not scale_x.
        scale = scale_x;
        scale_x = 1.0f;
    }
}

void LTScaleNode::draw() {
    if (child != NULL) {
        ltScale(scale_x * scale, scale_y * scale, scale_z * scale);
        child->draw();
    }
}

bool LTScaleNode::inverse_transform(LTfloat *x, LTfloat *y) {
    if (scale_x != 0.0f && scale_y != 0.0f && scale != 0.0f && scale_z == 1.0f) {
        *x /= (scale_x * scale);
        *y /= (scale_y * scale);
        return true;
    } else {
        return false;
    }
}

LT_REGISTER_TYPE(LTScaleNode, "lt.Scale", "lt.Wrap");
LT_REGISTER_FIELD_FLOAT(LTScaleNode, scale_x);
LT_REGISTER_FIELD_FLOAT(LTScaleNode, scale_y);
LT_REGISTER_FIELD_FLOAT(LTScaleNode, scale_z);
LT_REGISTER_FIELD_FLOAT(LTScaleNode, scale);

void LTShearNode::draw() {
    if (child != NULL) {
        LTfloat matrix[] = {
            1,  xy, xz, 0,
            yx, 1,  yz, 0,
            zx, zy, 1,  0,
            0,  0,  0,  1,
        };
        ltMultMatrix(matrix);
        child->draw();
    }
}

LT_REGISTER_TYPE(LTShearNode, "lt.Shear", "lt.Wrap");
LT_REGISTER_FIELD_FLOAT(LTShearNode, xy);
LT_REGISTER_FIELD_FLOAT(LTShearNode, xz);
LT_REGISTER_FIELD_FLOAT(LTShearNode, yx);
LT_REGISTER_FIELD_FLOAT(LTShearNode, yz);
LT_REGISTER_FIELD_FLOAT(LTShearNode, zx);
LT_REGISTER_FIELD_FLOAT(LTShearNode, zy);

LTTransformNode::LTTransformNode() {
    // initialize to identity
    m1 = 1;
    m6 = 1;
    m11 = 1;
    m16 = 1;
}

void LTTransformNode::draw() {
    if (child != NULL) {
        LTfloat matrix[] = {
            m1, m5, m9,  m13,
            m2, m6, m10, m14,
            m3, m7, m11, m15,
            m4, m8, m12, m16,
        };
        ltMultMatrix(matrix);
        child->draw();
    }
}

LT_REGISTER_TYPE(LTTransformNode, "lt.Transform", "lt.Wrap");
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m1);
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m2);
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m3);
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m4);
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m5);
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m6);
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m7);
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m8);
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m9);
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m10);
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m11);
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m12);
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m13);
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m14);
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m15);
LT_REGISTER_FIELD_FLOAT(LTTransformNode, m16);

void LTTintNode::draw() {
    if (child != NULL) {
        ltPushTint(red, green, blue, alpha);
        child->draw();
        ltPopTint();
    }
}

LT_REGISTER_TYPE(LTTintNode, "lt.Tint", "lt.Wrap");
LT_REGISTER_FIELD_FLOAT(LTTintNode, red);
LT_REGISTER_FIELD_FLOAT(LTTintNode, green);
LT_REGISTER_FIELD_FLOAT(LTTintNode, blue);
LT_REGISTER_FIELD_FLOAT(LTTintNode, alpha);

void LTTextureModeNode::draw() {
    if (child != NULL) {
        ltPushTextureMode(mode);
        child->draw();
        ltPopTextureMode();
    }
}

static const LTEnumConstant TextureMode_enum_vals[] = {
    {"modulate", LT_TEXTURE_MODE_MODULATE},
    {"add", LT_TEXTURE_MODE_ADD},
    {"decal", LT_TEXTURE_MODE_DECAL},
    {"blend", LT_TEXTURE_MODE_BLEND},
    {"replace", LT_TEXTURE_MODE_REPLACE},
    {NULL, 0}};
LT_REGISTER_TYPE(LTTextureModeNode, "lt.TextureMode", "lt.Wrap");
LT_REGISTER_FIELD_ENUM(LTTextureModeNode, mode, LTTextureMode, TextureMode_enum_vals);

static bool color_mask_red = true;
static bool color_mask_green = true;
static bool color_mask_blue = true;
static bool color_mask_alpha = true;

void LTColorMaskNode::draw() {
    if (child != NULL) {
        bool old_red = color_mask_red;
        bool old_green = color_mask_green;
        bool old_blue = color_mask_blue;
        bool old_alpha = color_mask_alpha;
        color_mask_red = red;
        color_mask_green = green;
        color_mask_blue = blue;
        color_mask_alpha = alpha;
        ltColorMask(red, green, blue, alpha);   
        child->draw();
        ltColorMask(old_red, old_green, old_blue, old_alpha);   
        color_mask_red = old_red;
        color_mask_green = old_green;
        color_mask_blue = old_blue;
        color_mask_alpha = old_alpha;
    }
}

LT_REGISTER_TYPE(LTColorMaskNode, "lt.ColorMask", "lt.Wrap");
LT_REGISTER_FIELD_BOOL(LTColorMaskNode, red);
LT_REGISTER_FIELD_BOOL(LTColorMaskNode, green);
LT_REGISTER_FIELD_BOOL(LTColorMaskNode, blue);
LT_REGISTER_FIELD_BOOL(LTColorMaskNode, alpha);

void LTBlendModeNode::draw() {
    if (child != NULL) {
        ltPushBlendMode(mode);
        child->draw();
        ltPopBlendMode();
    }
}

static const LTEnumConstant BlendMode_enum_vals[] = {
    {"normal", LT_BLEND_MODE_NORMAL},
    {"invert", LT_BLEND_MODE_INVERT},
    {"add", LT_BLEND_MODE_ADD},
    {"subtract", LT_BLEND_MODE_SUBTRACT},
    //{"diff", LT_BLEND_MODE_DIFF},
    {"color", LT_BLEND_MODE_COLOR},
    {"multiply", LT_BLEND_MODE_MULTIPLY},
    {"off", LT_BLEND_MODE_OFF},
    {NULL, 0}};
LT_REGISTER_TYPE(LTBlendModeNode, "lt.BlendMode", "lt.Wrap");
LT_REGISTER_FIELD_ENUM(LTBlendModeNode, mode, LTBlendMode, BlendMode_enum_vals);

void LTRectNode::draw() {
    ltDisableTextures();

    ltBindVertBuffer(0);
    LTfloat v[] = {
        x1, y1,
        x2, y1,
        x2, y2,
        x1, y2
    };
    ltVertexPointer(2, LT_VERT_DATA_TYPE_FLOAT, 0, v);
    ltDrawArrays(LT_DRAWMODE_TRIANGLE_FAN, 0, 4);
}

LT_REGISTER_TYPE(LTRectNode, "lt.Rect", "lt.SceneNode")
LT_REGISTER_FIELD_FLOAT(LTRectNode, x1)
LT_REGISTER_FIELD_FLOAT(LTRectNode, y1)
LT_REGISTER_FIELD_FLOAT(LTRectNode, x2)
LT_REGISTER_FIELD_FLOAT(LTRectNode, y2)

void LTHiddenNode::draw() {};

LT_REGISTER_TYPE(LTHiddenNode, "lt.Hidden", "lt.Wrap")

/*
static void check_scene_nodes() {
    std::list<LTSceneNode *>::iterator it;
    std::map<LTSceneNode *, int> active_nodes;
    std::set<LTSceneNode*>::iterator roots_it;
    CheckVisitor visitor(&active_nodes);
    for (roots_it = roots.begin(); roots_it != roots.end(); ++roots_it) {
        LTSceneNode *root = *roots_it;
        map_inc(&active_nodes, root);
        root->visit_children(&visitor);
    }
    for (it = all_nodes.begin(); it != all_nodes.end(); ++it) {
        if (active_nodes.find(*it) == active_nodes.end()) {
            assert((*it)->active == 0);
        } else {
            assert((*it)->active == active_nodes[*it]);
        }
    }
}
*/
