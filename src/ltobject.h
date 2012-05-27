/* Copyright (C) 2012 Ian MacLarty */

// Used for reflection.  Any subclasses of LTObject that can be used
// in Lua code should have an entry in this enumeration.
enum LTType {
    LT_TYPE_OBJECT,
    LT_TYPE_SCENENODE,
    LT_TYPE_TRANSLATE,
    LT_TYPE_ROTATE,
    LT_TYPE_SCALE,
    LT_TYPE_TINT,
    LT_TYPE_BLENDMODE,
    LT_TYPE_TEXTUREMODE,
    LT_TYPE_LAYER,
    LT_TYPE_TEXTUREDNODE,
    LT_TYPE_IMAGE,
    LT_TYPE_WORLD,
    LT_TYPE_BODY,
    LT_TYPE_FIXTURE,
    LT_TYPE_JOINT,
    LT_TYPE_LINE,
    LT_TYPE_TRIANGLE,
    LT_TYPE_RECT,
    LT_TYPE_CUBOID,
    LT_TYPE_PERSPECTIVE,
    LT_TYPE_PITCH,
    LT_TYPE_FOG,
    LT_TYPE_DEPTHTEST,
    LT_TYPE_DEPTHMASK,
    LT_TYPE_HITFILTER,
    LT_TYPE_DOWNFILTER,
    LT_TYPE_WRAP,
    LT_TYPE_AUDIOSAMPLE,
    LT_TYPE_TRACK,
    LT_TYPE_VECTOR,
    LT_TYPE_DRAWVECTOR,
    LT_TYPE_DRAWQUADS,
    LT_TYPE_BODYTRACKER,
    LT_TYPE_PARTICLESYSTEM,
    LT_TYPE_TWEENSET,
    LT_TYPE_RENDERTARGET,
    LT_TYPE_RANDOMGENERATOR,
    LT_NUM_TYPES
};

const char* ltTypeName(LTType type);

/*
 * Values below LT_FIELD_TYPE_START_VAL correspond to a LTType type.
 */
#define LT_FIELD_TYPE_START_VAL 10000
enum LTFieldType {
    LT_FIELD_TYPE_FLOAT = LT_FIELD_TYPE_START_VAL,
    LT_FIELD_TYPE_INT,
    LT_FIELD_TYPE_BOOL,
    LT_FIELD_TYPE_LUA_REF,
};

enum LTFieldAccess {
    LT_ACCESS_FULL,
    LT_ACCESS_READONLY,
};

struct LTFieldDescriptor {
    const char *name;
    int type; // Either a LTType or a LTFieldType.
    int offset;
    void* getter;
    void* setter; 
    LTFieldAccess access;
};

#define LT_END_FIELD_DESCRIPTOR_LIST {NULL, LT_FIELD_TYPE_INT, 0, NULL, NULL, LT_ACCESS_READONLY}
#define LT_OFFSETOF(f) ((int)((char*)&(f) - (char*)this))

struct LTObject;

typedef LTfloat     (*LTFloatGetter)(LTObject*);
typedef void        (*LTFloatSetter)(LTObject*, LTfloat);
typedef LTint       (*LTIntGetter)(LTObject*);
typedef void        (*LTIntSetter)(LTObject*, LTint);
typedef LTbool      (*LTBoolGetter)(LTObject*);
typedef void        (*LTBoolSetter)(LTObject*, LTbool);
typedef LTObject*   (*LTObjGetter)(LTObject*);
typedef void        (*LTObjSetter)(LTObject*, LTObject*);

struct LTObject {
    int lua_wrap; // Reference to Lua wrapper table.
    LTType type;

    LTObject(LTType type);
    virtual ~LTObject();

    // name must be a Lua string.
    LTFieldDescriptor *field(const char *name);
    virtual LTFieldDescriptor *fields();

    inline LTfloat getFloatField(LTFieldDescriptor *field) {
        if (field->offset >= 0) {
            return *((LTfloat*)((char*)this + field->offset));
        } else {
            LTFloatGetter getter = (LTFloatGetter)field->getter;
            return getter(this);
        }
    }

    inline void setFloatField(LTFieldDescriptor *field, LTfloat val) {
        if (field->access == LT_ACCESS_FULL) {
            if (field->offset >= 0) {
                *((LTfloat*)((char*)this + field->offset)) = val;
            } else {
                LTFloatSetter setter = (LTFloatSetter)field->setter;
                setter(this, val);
            }
        }
    }

    inline LTint getIntField(LTFieldDescriptor *field) {
        if (field->offset >= 0) {
            return *((LTint*)((char*)this + field->offset));
        } else {
            LTIntGetter getter = (LTIntGetter)field->getter;
            return getter(this);
        }
    }

    inline void setIntField(LTFieldDescriptor *field, LTint val) {
        if (field->access == LT_ACCESS_FULL) {
            if (field->offset >= 0) {
                *((LTint*)((char*)this + field->offset)) = val;
            } else {
                LTIntSetter setter = (LTIntSetter)field->setter;
                setter(this, val);
            }
        }
    }

    inline LTbool getBoolField(LTFieldDescriptor *field) {
        if (field->offset >= 0) {
            return *((LTbool*)((char*)this + field->offset));
        } else {
            LTBoolGetter getter = (LTBoolGetter)field->getter;
            return getter(this);
        }
    }

    inline void setBoolField(LTFieldDescriptor *field, LTbool val) {
        if (field->access == LT_ACCESS_FULL) {
            if (field->offset >= 0) {
                *((LTbool*)((char*)this + field->offset)) = val;
            } else {
                LTBoolSetter setter = (LTBoolSetter)field->setter;
                setter(this, val);
            }
        }
    }

    inline LTObject *getObjField(LTFieldDescriptor *field) {
        if (field->offset >= 0) {
            return *((LTObject**)((char*)this + field->offset));
        } else {
            LTObjGetter getter = (LTObjGetter)field->getter;
            return getter(this);
        }
    }

    inline void setObjField(LTFieldDescriptor *field, LTObject *val) {
        if (field->access == LT_ACCESS_FULL) {
            if (field->offset >= 0) {
                *((LTObject**)((char*)this + field->offset)) = val;
            } else {
                LTObjSetter setter = (LTObjSetter)field->setter;
                setter(this, val);
            }
        }
    }

    bool hasType(LTType t);
    const char* typeName();
};

void ltInitObjectFieldCache();
