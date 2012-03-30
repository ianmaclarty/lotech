/* Copyright (C) 2012 Ian MacLarty */
#ifndef LTOBJECT_H
#define LTOBJECT_H

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

enum LTFieldType {
    LT_FIELD_TYPE_FLOAT,
    LT_FIELD_TYPE_INT,
    LT_FIELD_TYPE_BOOL,
};

enum LTFieldAccess {
    LT_ACCESS_FULL,
    LT_ACCESS_READONLY,
};

struct LTFieldDescriptor {
    const char *name;
    LTFieldType type;
    int offset;
    void* getter;
    void* setter; 
    LTFieldAccess access;
};

#define LT_END_FIELD_DESCRIPTOR_LIST {NULL, LT_FIELD_TYPE_INT, 0, NULL, NULL, LT_ACCESS_READONLY}
#define LT_OFFSETOF(f) ((int)((char*)&f - (char*)this))

struct LTObject;

typedef LTfloat (ltFloatGetter)(LTObject*);
typedef void    (ltFloatSetter)(LTObject*, LTfloat);
typedef LTint   (ltIntGetter)(LTObject*);
typedef void    (ltIntSetter)(LTObject*, LTint);
typedef LTbool  (ltBoolGetter)(LTObject*);
typedef void    (ltBoolSetter)(LTObject*, LTbool);

struct LTObject {
    int lua_wrap; // Reference to Lua wrapper table.
    LTType type;

    LTObject(LTType type);
    virtual ~LTObject();

    // These are deprecated...
    virtual bool has_field(const char *field_name);
    virtual LTfloat get_field(const char *field_name);
    virtual void set_field(const char *field_name, LTfloat value);
    virtual LTfloat* field_ptr(const char *field_name);

    // ... in favour of this:
    // name must be a Lua string.
    LTFieldDescriptor *field(const char *name);
    virtual const LTFieldDescriptor *fields();

    bool hasType(LTType t);
    const char* typeName();
};

void ltInitObjectFieldCache();

#endif
