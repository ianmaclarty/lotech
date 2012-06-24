/* Copyright (C) 2010 Ian MacLarty */

#include "lt.h"

LT_INIT_IMPL(ltscene)

LTSceneNode::LTSceneNode() {
    event_handlers = NULL;
    active = 0;
}

LTSceneNode::~LTSceneNode() {
    if (event_handlers != NULL) {
        std::list<LTPointerEventHandler*>::iterator it;
        for (it = event_handlers->begin(); it != event_handlers->end(); it++) {
            delete *it;
        }
        delete event_handlers;
    }
    if (actions != NULL) {
        std::list<LTAction*>::iterator it;
        for (it = actions->begin(); it != actions->end(); it++) {
            delete *it;
        }
        delete actions;
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

struct EnterVisitor : LTSceneNodeVisitor {
    LTSceneNode *parent;
    EnterVisitor(LTSceneNode *p) {
        parent = p;
    }
    virtual void visit(LTSceneNode *node) {
        node->enter(parent);
    }
};

void LTSceneNode::enter(LTSceneNode *parent) {
    if (parent == NULL || parent->active) {
        if (!active) {
            on_activate();
            if (actions != NULL) {
                for (std::list<LTAction*>::iterator it = actions->begin(); it != actions->end(); it++) {
                    (*it)->schedule();
                }
            }
        }
        active++;
        EnterVisitor v(this);
        visitChildren(&v);
    }
}

struct ExitVisitor : LTSceneNodeVisitor {
    LTSceneNode *parent;
    ExitVisitor(LTSceneNode *p) {
        parent = p;
    }
    virtual void visit(LTSceneNode *node) {
        node->exit(parent);
    }
};

void LTSceneNode::exit(LTSceneNode *parent) {
    if (parent == NULL || parent->active) {
        ExitVisitor v(this);
        visitChildren(&v);
        active--;
        if (!active) {
            if (actions != NULL) {
                for (std::list<LTAction*>::iterator it = actions->begin(); it != actions->end(); it++) {
                    (*it)->unschedule();
                }
            }
            on_deactivate();
        }
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
        return 0;
    } else {
        return luaL_error(L, "Scene node already active");
    }
}

static int deactivate_scene_node(lua_State *L) {
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
        return 0;
    } else {
        return luaL_error(L, "Scene node not active");
    }
}

LT_REGISTER_METHOD(LTSceneNode, Activate, activate_scene_node);
LT_REGISTER_METHOD(LTSceneNode, Deactivate, deactivate_scene_node);

LTWrapNode::LTWrapNode() {
    LTWrapNode::child = NULL;
}

void LTWrapNode::init(lua_State *L) {
    LTSceneNode::init(L);
    if (child == NULL) {
        luaL_error(L, "No child element for wrap node");
    }
}

void LTWrapNode::draw() {
    child->draw();
}

void LTWrapNode::visitChildren(LTSceneNodeVisitor *v) {
    v->visit(child);
}

bool LTWrapNode::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (!consumePointerEvent(x, y, event)) {
        return child->propogatePointerEvent(x, y, event);
    } else {
        return true;
    }
}

static LTObject *get_child(LTObject *obj) {
    return ((LTWrapNode*)obj)->child;
}

static void set_child(LTObject *obj, LTObject *val) {
    LTWrapNode *wrap = (LTWrapNode*)obj;
    LTSceneNode *child = (LTSceneNode*)val;
    if (wrap->child != NULL) {
        wrap->child->exit(wrap);
    }
    wrap->child = child;
    child->enter(wrap);
}

LT_REGISTER_TYPE(LTWrapNode, "lt.Wrap", "lt.SceneNode");
LT_REGISTER_PROPERTY_OBJ(LTWrapNode, child, LTSceneNode, get_child, set_child);

#define NODEINDEX std::list<LTSceneNode*>::iterator

void LTLayer::insert_front(LTSceneNode *node, int ref) {
    node_list.push_back(LTLayerNodeRefPair(node, ref));
    node_index.insert(std::pair<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>(node, --node_list.end()));
    node->enter(this);
}

void LTLayer::insert_back(LTSceneNode *node, int ref) {
    node_list.push_front(LTLayerNodeRefPair(node, ref));
    node_index.insert(std::pair<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>(node, node_list.begin()));
    node->enter(this);
}

bool LTLayer::insert_above(LTSceneNode *existing_node, LTSceneNode *new_node, int ref) {
    std::pair<std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>::iterator,
              std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>::iterator> range;
    std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>::iterator it;
    range = node_index.equal_range(existing_node);
    if (range.first != range.second) {
        std::list<LTLayerNodeRefPair>::iterator existing_it = (range.first)->second;
        std::list<LTLayerNodeRefPair>::iterator new_it = node_list.insert(++existing_it, LTLayerNodeRefPair(new_node, ref));
        node_index.insert(std::pair<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>(new_node, new_it));
        new_node->enter(this);
        return true;
    } else {
        return false;
    }
}

bool LTLayer::insert_below(LTSceneNode *existing_node, LTSceneNode *new_node, int ref) {
    std::pair<std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>::iterator,
              std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>::iterator> range;
    std::multimap<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>::iterator it;
    range = node_index.equal_range(existing_node);
    if (range.first != range.second) {
        std::list<LTLayerNodeRefPair>::iterator existing_it = (range.first)->second;
        std::list<LTLayerNodeRefPair>::iterator new_it = node_list.insert(existing_it, LTLayerNodeRefPair(new_node, ref));
        node_index.insert(std::pair<LTSceneNode*, std::list<LTLayerNodeRefPair>::iterator>(new_node, new_it));
        new_node->enter(this);
        return true;
    } else {
        return false;
    }
}

int LTLayer::size() {
    return node_list.size();
}

void LTLayer::remove(lua_State *L, int layer_index, LTSceneNode *node) {
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

void LTLayer::visitChildren(LTSceneNodeVisitor *v) {
    std::list<LTLayerNodeRefPair>::iterator it;
    for (it = node_list.begin(); it != node_list.end(); it++) {
        v->visit((*it).node);
    }
}

bool LTLayer::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (!consumePointerEvent(x, y, event)) {
        std::list<LTLayerNodeRefPair>::reverse_iterator it;
        for (it = node_list.rbegin(); it != node_list.rend(); it++) {
            if ((*it).node->propogatePointerEvent(x, y, event)) {
                return true;
            }
        }
        return false;
    } else {
        return true;
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
    ltTranslate(x, y, z);
    child->draw();
}

bool LTTranslateNode::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (!consumePointerEvent(x, y, event)) {
        LTfloat x1, y1;
        x1 = x - LTTranslateNode::x;
        y1 = y - LTTranslateNode::y;
        if (z == 0.0f) {
            return child->propogatePointerEvent(x1, y1, event);
        } else {
            return false;
        }
    } else {
        return true;
    }
}

LT_REGISTER_TYPE(LTTranslateNode, "lt.Translate", "lt.Wrap");
LT_REGISTER_FIELD_FLOAT(LTTranslateNode, x);
LT_REGISTER_FIELD_FLOAT(LTTranslateNode, y);
LT_REGISTER_FIELD_FLOAT(LTTranslateNode, z);

void LTRotateNode::draw() {
    ltTranslate(cx, cy, 0.0f);
    ltRotate(angle, 0.0f, 0.0f, 1.0f);
    ltTranslate(-cx, -cy, 0.0f);
    child->draw();
}

bool LTRotateNode::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (!consumePointerEvent(x, y, event)) {
        LTfloat x1, y1;
        LTfloat a = -angle * LT_RADIANS_PER_DEGREE;
        LTfloat s = sinf(a);
        LTfloat c = cosf(a);
        x1 = c * x - s * y;
        y1 = s * x + c * y;
        return child->propogatePointerEvent(x1, y1, event);
    } else {
        return true;
    }
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
    ltScale(scale_x * scale, scale_y * scale, scale_z * scale);
    child->draw();
}

bool LTScaleNode::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (!consumePointerEvent(x, y, event)) {
        LTfloat x1, y1;
        if (scale_x != 0.0f && scale_y != 0.0f && scale != 0.0f && scale_z == 1.0f) {
            x1 = x / (scale_x * scale);
            y1 = y / (scale_y * scale);
            return child->propogatePointerEvent(x1, y1, event);
        } else {
            return false;
        }
    } else {
        return true;
    }
}

LT_REGISTER_TYPE(LTScaleNode, "lt.Scale", "lt.Wrap");
LT_REGISTER_FIELD_FLOAT(LTScaleNode, scale_x);
LT_REGISTER_FIELD_FLOAT(LTScaleNode, scale_y);
LT_REGISTER_FIELD_FLOAT(LTScaleNode, scale_z);
LT_REGISTER_FIELD_FLOAT(LTScaleNode, scale);

void LTTintNode::draw() {
    ltPushTint(red, green, blue, alpha);
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

LT_REGISTER_TYPE(LTTintNode, "lt.Tint", "lt.Wrap");
LT_REGISTER_FIELD_FLOAT(LTTintNode, red);
LT_REGISTER_FIELD_FLOAT(LTTintNode, green);
LT_REGISTER_FIELD_FLOAT(LTTintNode, blue);
LT_REGISTER_FIELD_FLOAT(LTTintNode, alpha);

void LTTextureModeNode::draw() {
    ltPushTextureMode(mode);
    child->draw();
    ltPopTextureMode();
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

void LTBlendModeNode::draw() {
    ltPushBlendMode(mode);
    child->draw();
    ltPopBlendMode();
}

static const LTEnumConstant BlendMode_enum_vals[] = {
    {"normal", LT_BLEND_MODE_NORMAL},
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

LT_REGISTER_TYPE(LTHitFilter, "lt.HitFilter", "lt.Wrap")
LT_REGISTER_FIELD_FLOAT(LTHitFilter, left)
LT_REGISTER_FIELD_FLOAT(LTHitFilter, bottom)
LT_REGISTER_FIELD_FLOAT(LTHitFilter, right)
LT_REGISTER_FIELD_FLOAT(LTHitFilter, top)

void LTDownFilter::draw() {
    child->draw();
}

bool LTDownFilter::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (event->type != LT_EVENT_POINTER_DOWN || (x >= left && x <= right && y >= bottom && y <= top)) {
        if (!consumePointerEvent(x, y, event)) {
            return child->propogatePointerEvent(x, y, event);
        } else {
            return true;
        }
    } else {
        return false;
    }
}

LT_REGISTER_TYPE(LTDownFilter, "lt.DownFilter", "lt.Wrap")
LT_REGISTER_FIELD_FLOAT(LTDownFilter, left)
LT_REGISTER_FIELD_FLOAT(LTDownFilter, bottom)
LT_REGISTER_FIELD_FLOAT(LTDownFilter, right)
LT_REGISTER_FIELD_FLOAT(LTDownFilter, top)
