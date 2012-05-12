/* Copyright (C) 2010 Ian MacLarty */
#include "lt.h"

struct LTFieldCache;

static inline LTFieldDescriptor *lookup_field(const char *name, LTFieldCache *cache);
static void init_type_parent();
static void init_field_cache_2();

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
    init_type_parent();
    init_field_cache_2();
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

//--------------

static const LTFieldDescriptor no_fields[] = {LT_END_FIELD_DESCRIPTOR_LIST};
const LTTypeDef lt_typedef_int =        {"int",     NULL, sizeof(LTint),    NULL, NULL, no_fields};
const LTTypeDef lt_typedef_float =      {"float",   NULL, sizeof(LTfloat),  NULL, NULL, no_fields};
const LTTypeDef lt_typedef_bool =       {"bool",    NULL, sizeof(LTbool),   NULL, NULL, no_fields};
const LTTypeDef lt_typedef_lua_ref =    {"lua_ref", NULL, sizeof(int),      NULL, NULL, no_fields};
const LTTypeDef lt_typedef_Object =     {"Object",  NULL, sizeof(LTObject), NULL, NULL, no_fields};

//--------------

static std::vector<const LTTypeDef*> *type_registry;

static void register_type(const LTTypeDef *type) {
    type_registry->push_back(type);
    fprintf(stderr, "Registered type %s, parent = %s, size = %d\n", type->name, type->parent, type->size);
}

LTRegisterType::LTRegisterType(const LTTypeDef *type) {
    if (type_registry == NULL) {
        type_registry = new std::vector<const LTTypeDef*>();
    }
    register_type(type);
}

//--------------

static std::vector<int> type_parent;

static bool str_cmp(const char *a, const char *b) {
    return strcmp(a, b) < 0;
}

static void init_type_parent() {
    std::map<const char *, int, bool (*)(const char*, const char*)> type_by_name(&str_cmp);
    int n = type_registry->size();
    const char* name;
    const char* parent;
    type_parent.resize(n);
    for (int i = 0; i < n; i++) {
        name = (*type_registry)[i]->name;
        if (type_by_name.find(name) != type_by_name.end()) {
            ltLog("Error: Duplicate entries for type %s", name);
            ltAbort();
        }
        type_by_name[name] = i;
    }
    std::map<const char *, int, bool (*)(const char*, const char*)>::iterator it;
    for (int i = 0; i < n; i++) {
        name = (*type_registry)[i]->name;
        parent = (*type_registry)[i]->parent;
        if (parent == NULL || (strcmp(parent, "Object") == 0)) {
            type_parent[i] = -1;
        } else {
            it = type_by_name.find(parent);
            if (it == type_by_name.end()) {
                ltLog("Error: Parent %s for type %s not registered", parent, name);
                ltAbort();
            } else {
                type_parent[i] = it->second;
            }
        }
    }
}

//--------------

struct LTFieldCache {
    int num_fields;
    const void* entries[]; // First num_fields entries are lua interned field name strings
                           // Then a NULL sentinal.
                           // Then next num_fields entries are pointers to the LTFieldDescriptors.
};

static std::vector<LTFieldCache *> field_cache_2;

// Counts fields in type i and all its ancestors.  Duplicates
// (i.e. fields with the same name) are not counted.
static int count_fields(int i) {
    std::set<const char *, bool (*)(const char*, const char*)> seen(str_cmp);
    int num_fields = 0;
    while (i >= 0) {
        const LTTypeDef *type = (*type_registry)[i];
        const LTFieldDescriptor *fields = type->fields;
        const LTFieldDescriptor *ptr = fields;
        while (ptr->name != NULL) {
            if (seen.find(ptr->name) == seen.end()) {
                num_fields++;
                seen.insert(ptr->name);
            }
            ptr++;
        }
        i = type_parent[i];
    }
    return num_fields;
}

static void init_field_cache_2() {
    int n = type_registry->size();
    for (unsigned int i = 0; i < field_cache_2.size(); i++) {
        if (field_cache_2[i] == NULL) {
            ltLog("Internal error: field_cache_2[%d] == NULL", i);
            ltAbort();
        }
        free(field_cache_2[i]);
        field_cache_2[i] = NULL;
    }
    field_cache_2.resize(n);
    for (int i = 0; i < n; i++) {
        int m = count_fields(i);
        LTFieldCache *cache = (LTFieldCache*)malloc(sizeof(LTFieldCache) + sizeof(void*) * (m + 1 + m));
        cache->num_fields = m;
        cache->entries[m] = NULL; // sentinal
        std::set<const char *, bool (*)(const char*, const char*)> seen(str_cmp);
        int k = 0;
        int j = i;
        while (j >= 0) {
            const LTTypeDef *type = (*type_registry)[j];
            const LTFieldDescriptor *fields = type->fields;
            const LTFieldDescriptor *ptr = fields;
            while (ptr->name != NULL) {
                if (seen.find(ptr->name) == seen.end()) {
                    seen.insert(ptr->name);
                    const char *lstr = ltLuaCacheString(ptr->name);
                    cache->entries[k] = lstr;
                    cache->entries[k + m + 1] = ptr;
                    k++;
                }
                ptr++;
            }
            j = type_parent[j];
        }
        field_cache_2[i] = cache;
    }
}

static inline LTFieldDescriptor *lookup_field(const char *name, LTFieldCache *cache) {
    const char **entry = (const char **)cache->entries;
    while (*entry != NULL && *entry != name) {
        entry++;
    }
    if (entry != NULL) {
        return *((LTFieldDescriptor**)(entry + cache->num_fields + 1));
    } else {
        return NULL; // Field not found.
    }
}
