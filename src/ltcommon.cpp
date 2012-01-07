/* Copyright (C) 2010 Ian MacLarty */
#include "ltcommon.h"
#include "ltlua.h"

ct_assert(sizeof(int) == 4);
ct_assert(sizeof(LTuint32) == 4);
ct_assert(sizeof(LTfloat) == 4);
ct_assert(sizeof(GLfloat) == 4);
ct_assert(sizeof(float) == 4);
ct_assert(sizeof(LTdouble) == 8);

//-----------------------------------------------------------------------
// LTObject.

struct LTTypeInfo {
    const char *name;
    LTType super_type;
};

const LTTypeInfo types[] = {
    {"Object",          LT_TYPE_OBJECT},
    {"SceneNode",       LT_TYPE_OBJECT},
    {"Translate",       LT_TYPE_WRAP},  
    {"Rotate",          LT_TYPE_WRAP},  
    {"Scale",           LT_TYPE_WRAP},  
    {"Tint",            LT_TYPE_WRAP},  
    {"BlendMode",       LT_TYPE_WRAP},  
    {"TextureMode",     LT_TYPE_WRAP},  
    {"Layer",           LT_TYPE_SCENENODE},  
    {"Image",           LT_TYPE_SCENENODE},  
    {"World",           LT_TYPE_OBJECT},
    {"Body",            LT_TYPE_SCENENODE},
    {"Fixture",         LT_TYPE_SCENENODE},
    {"Joint",           LT_TYPE_JOINT},
    {"Line",            LT_TYPE_SCENENODE},
    {"Triangle",        LT_TYPE_SCENENODE},
    {"Rect",            LT_TYPE_SCENENODE},
    {"Cuboid",          LT_TYPE_SCENENODE},
    {"Perspective",     LT_TYPE_WRAP},
    {"Pitch",           LT_TYPE_WRAP},
    {"Fog",             LT_TYPE_WRAP},
    {"DepthTest",       LT_TYPE_WRAP},
    {"HitFilter",       LT_TYPE_WRAP},
    {"DownFilter",      LT_TYPE_WRAP},
    {"Wrap",            LT_TYPE_SCENENODE},
    {"Sample",          LT_TYPE_OBJECT},
    {"Track",           LT_TYPE_OBJECT},
    {"Vector",          LT_TYPE_OBJECT},
    {"DrawVector",      LT_TYPE_SCENENODE},
    {"DrawQuads",       LT_TYPE_SCENENODE},
    {"BodyTracker",     LT_TYPE_WRAP},
    {"ParticleSystem",  LT_TYPE_SCENENODE},
    {"TweenSet",        LT_TYPE_OBJECT},
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

bool LTObject::has_field(const char *field_name) {
    return field_ptr(field_name) != NULL;
}

LTfloat LTObject::get_field(const char *field_name) {
    LTfloat *ptr = field_ptr(field_name);
    if (ptr != NULL) {
        return *ptr;
    } else {
        return 0.0f;
    }
}

void LTObject::set_field(const char *field_name, LTfloat value) {
    LTfloat *ptr = field_ptr(field_name);
    if (ptr != NULL) {
        *ptr = value;
    }
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
