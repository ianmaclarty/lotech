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

struct LTObject {
    int lua_wrap; // Reference to Lua wrapper table.
    LTType type;

    LTObject(LTType type);
    virtual ~LTObject();

    // For tweening and modification from lua.
    virtual bool has_field(const char *field_name);
    virtual LTfloat get_field(const char *field_name);
    virtual void set_field(const char *field_name, LTfloat value);
    virtual LTfloat* field_ptr(const char *field_name);

    // Is this object of a certain type?
    bool hasType(LTType t);

    const char* typeName();
};

#endif
