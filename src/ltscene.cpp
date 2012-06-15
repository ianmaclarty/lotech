/* Copyright (C) 2010 Ian MacLarty */

#include "lt.h"

LT_INIT_IMPL(ltscene)

int lt_curr_advance_step = 0;

LTSceneNode::LTSceneNode() {
    event_handlers = NULL;
    actions = NULL;
    last_advance_step = -1;
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

void LTSceneNode::advance(LTfloat dt) {
    if (!executeActions(dt)) return;
}

bool LTSceneNode::executeActions(LTfloat dt) {
    if (last_advance_step != lt_curr_advance_step) {
        if (actions != NULL) {
            std::list<LTAction*>::iterator it = actions->begin();
            while (it != actions->end()) {
                if ((*it)->doAction(dt, this)) {
                    delete *it;
                    it = actions->erase(it);
                } else {
                    it++;
                }
            }
        }
        last_advance_step = lt_curr_advance_step;
        return true;
    } else {
        return false;
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

LT_REGISTER_TYPE(LTSceneNode, "lt.SceneNode", "lt.Object");

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

void LTWrapNode::advance(LTfloat dt) {
    if (!executeActions(dt)) return;
    child->advance(dt);
}

bool LTWrapNode::propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event) {
    if (!consumePointerEvent(x, y, event)) {
        return child->propogatePointerEvent(x, y, event);
    } else {
        return true;
    }
}

LT_REGISTER_TYPE(LTWrapNode, "lt.Wrap", "lt.SceneNode");
LT_REGISTER_FIELD_OBJ(LTWrapNode, child, LTSceneNode);

#define NODEINDEX std::list<LTSceneNode*>::iterator

void LTLayer::insert_front(LTSceneNode *node) {
    node_list.push_back(node);
    node_index.insert(std::pair<LTSceneNode*, std::list<LTSceneNode*>::iterator>(node, --node_list.end()));
}

void LTLayer::insert_back(LTSceneNode *node) {
    node_list.push_front(node);
    node_index.insert(std::pair<LTSceneNode*, std::list<LTSceneNode*>::iterator>(node, node_list.begin()));
}

bool LTLayer::insert_above(LTSceneNode *existing_node, LTSceneNode *new_node) {
    std::pair<std::multimap<LTSceneNode*, std::list<LTSceneNode*>::iterator>::iterator,
              std::multimap<LTSceneNode*, std::list<LTSceneNode*>::iterator>::iterator> range;
    std::multimap<LTSceneNode*, std::list<LTSceneNode*>::iterator>::iterator it;
    range = node_index.equal_range(existing_node);
    if (range.first != range.second) {
        std::list<LTSceneNode*>::iterator existing_it = (range.first)->second;
        std::list<LTSceneNode*>::iterator new_it = node_list.insert(++existing_it, new_node);
        node_index.insert(std::pair<LTSceneNode*, std::list<LTSceneNode*>::iterator>(new_node, new_it));
        return true;
    } else {
        return false;
    }
}

bool LTLayer::insert_below(LTSceneNode *existing_node, LTSceneNode *new_node) {
    std::pair<std::multimap<LTSceneNode*, std::list<LTSceneNode*>::iterator>::iterator,
              std::multimap<LTSceneNode*, std::list<LTSceneNode*>::iterator>::iterator> range;
    std::multimap<LTSceneNode*, std::list<LTSceneNode*>::iterator>::iterator it;
    range = node_index.equal_range(existing_node);
    if (range.first != range.second) {
        std::list<LTSceneNode*>::iterator existing_it = (range.first)->second;
        std::list<LTSceneNode*>::iterator new_it = node_list.insert(existing_it, new_node);
        node_index.insert(std::pair<LTSceneNode*, std::list<LTSceneNode*>::iterator>(new_node, new_it));
        return true;
    } else {
        return false;
    }
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

void LTLayer::advance(LTfloat dt) {
    if (!executeActions(dt)) return;
    std::list<LTSceneNode*>::iterator it;
    for (it = node_list.begin(); it != node_list.end(); it++) {
        (*it)->advance(dt);
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

LT_REGISTER_TYPE(LTLayer, "lt.Layer", "lt.SceneNode");

static int new_Layer(lua_State *L) {
    int num_args = ltLuaCheckNArgs(L, 0);
    LTLayer *layer = new (lt_alloc_LTLayer(L)) LTLayer();
    // Add arguments as child nodes.
    // First arguments are drawn in front of last arguments.
    for (int arg = 1; arg <= num_args; arg++) {
        LTSceneNode *child = lt_expect_LTSceneNode(L, arg);
        layer->insert_back(child);
        ltLuaAddRef(L, -1, arg); // Add reference from layer node to child node.
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
