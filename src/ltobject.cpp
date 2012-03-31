/* Copyright (C) 2010 Ian MacLarty */
#include "lt.h"

struct LTTypeInfo {
    const char *name;
    LTType super_type;
};

static const LTTypeInfo types[] = {
    {"Object",          LT_TYPE_OBJECT},
    {"SceneNode",       LT_TYPE_OBJECT},
    {"Translate",       LT_TYPE_WRAP},  
    {"Rotate",          LT_TYPE_WRAP},  
    {"Scale",           LT_TYPE_WRAP},  
    {"Tint",            LT_TYPE_WRAP},  
    {"BlendMode",       LT_TYPE_WRAP},  
    {"TextureMode",     LT_TYPE_WRAP},  
    {"Layer",           LT_TYPE_SCENENODE},  
    {"TexturedNode",    LT_TYPE_SCENENODE},  
    {"Image",           LT_TYPE_TEXTUREDNODE},  
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
    {"DepthMask",       LT_TYPE_WRAP},
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
    {"RenderTarget",    LT_TYPE_SCENENODE},
    {"RandomGenerator", LT_TYPE_OBJECT},
};

ct_assert(sizeof(types) == (int)LT_NUM_TYPES * sizeof(LTTypeInfo));

const char* ltTypeName(LTType type) {
    return types[type].name;
}

LTObject::LTObject(LTType type) {
    lua_wrap = LUA_NOREF;
    LTObject::type = type;
}

LTObject::~LTObject() {
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

//-------------- field access ---------------------

static LTFieldDescriptor* field_cache[LT_NUM_TYPES];

// This is called whenever the Lua engine is (re)initialized.
void ltInitObjectFieldCache() {
    for (int i = 0; i < LT_NUM_TYPES; i++) {
        if (field_cache[i] != NULL) {
            delete[] field_cache[i];
        }
        field_cache[i] = NULL;
    }
}

// This is just used to initialize field_cache the first time.
struct FieldCacheInit {
    FieldCacheInit() {
        for (int i = 0; i < LT_NUM_TYPES; i++) {
            field_cache[i] = NULL;
        }
    }
};

static FieldCacheInit init_field_cache;

LTFieldDescriptor* LTObject::field(const char *name) {
    LTFieldDescriptor *cache = field_cache[type];
    if (cache != NULL) {
        while (cache->name != NULL) {
            if (cache->name == name) {
                return cache;
            }
            cache++;
        }
        return NULL;
    } else {
        // Cache not set up for this object type.
        LTFieldDescriptor *flds = fields();
        LTFieldDescriptor *ptr = flds;
        int num_fields = 0;
        while (ptr->name != NULL) {
            num_fields++;
            ptr++;
        }
        cache = new LTFieldDescriptor[num_fields + 1];
        cache[num_fields].name = NULL;
        for (int i = 0; i < num_fields; i++) {
            const char *lstr = ltLuaCacheString(flds[i].name);
            cache[i] = flds[i];
            cache[i].name = lstr;
        }
        field_cache[type] = cache;
        return field(name);
    }
}

LTFieldDescriptor* LTObject::fields() {
    static LTFieldDescriptor flds[] = {
        LT_END_FIELD_DESCRIPTOR_LIST
    };
    return flds;
}
