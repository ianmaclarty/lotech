/* Copyright (C) 2010 Ian MacLarty */
#include "ltcommon.h"
#include "ltlua.h"

//-----------------------------------------------------------------------
// LTObject.

struct LTTypeInfo {
    const char *name;
    LTType super_type;
};

const LTTypeInfo types[] = {
    {"Object",      LT_TYPE_OBJECT},
    {"SceneNode",   LT_TYPE_OBJECT},
    {"Translate",   LT_TYPE_SCENENODE},  
    {"Rotate",      LT_TYPE_SCENENODE},  
    {"Scale",       LT_TYPE_SCENENODE},  
    {"Tint",        LT_TYPE_SCENENODE},  
    {"Layer",       LT_TYPE_SCENENODE},  
    {"Image",       LT_TYPE_SCENENODE},  
    {"Atlas",       LT_TYPE_OBJECT},
    {"World",       LT_TYPE_OBJECT},
    {"Body",        LT_TYPE_SCENENODE},
    {"Fixture",     LT_TYPE_SCENENODE},
    {"Line",        LT_TYPE_SCENENODE},
    {"Triangle",    LT_TYPE_SCENENODE},
    {"Rect",        LT_TYPE_SCENENODE},
    {"Cuboid",      LT_TYPE_SCENENODE},
    {"Perspective", LT_TYPE_SCENENODE},
};

ct_assert(sizeof(types) == (int)LT_NUM_TYPES * sizeof(LTTypeInfo));

const char* ltTypeName(LTType type) {
    return types[type].name;
}

LTObject::LTObject(LTType type) {
    lua_wrap = ltLuaInitRef();
    LTObject::type = type;
}

LTObject::~LTObject() {
}

LTfloat* LTObject::field_ptr(const char *field_name) {
    return NULL;
}

bool LTObject::hasType(LTType t) {
    if (t == LT_TYPE_OBJECT) {
        return true;
    }
    LTType t1 = type;
    while (t1 != LT_TYPE_OBJECT) {
        if (t == t1) {
            return true;
        }
        t1 = types[t1].super_type;
    }
    return false;
}

const char* LTObject::typeName() {
    return ltTypeName(type);
}
