/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTCOMMON_H
#define LTCOMMON_H

#ifdef LTLINUX
    #define GL_GLEXT_PROTOTYPES
    #include <GL/gl.h>
    #include <GL/glext.h>
#else
    #ifdef LTIOS
        #include <OpenGLES/ES1/gl.h>
        #include <OpenGLES/ES1/glext.h>
    #else
        #include <OpenGL/GL.h>
    #endif
#endif

#include <stdlib.h>
#include <stdio.h>

#include "ltutil.h"

#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
#define ct_assert(e) enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }

#define LT_PI 3.14159265358979323846f
#define LT_RADIANS_PER_DEGREE (LT_PI / 180.0f)
#define LT_DEGREES_PER_RADIAN (180.0f / LT_PI)

typedef float           LTfloat;
typedef unsigned int    LTuint;
typedef float           LTsecs;
typedef float           LTdegrees;
typedef unsigned int    LTuint32;

typedef LTuint32        LTpixel;
typedef GLuint          LTvertbuf;
typedef GLuint          LTtexbuf;

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
    LT_TYPE_LAYER,
    LT_TYPE_IMAGE,
    LT_TYPE_ATLAS,
    LT_TYPE_WORLD,
    LT_TYPE_BODY,
    LT_TYPE_FIXTURE,
    LT_TYPE_LINE,
    LT_TYPE_TRIANGLE,
    LT_TYPE_RECT,
    LT_TYPE_CUBOID,
    LT_TYPE_PERSPECTIVE,
    LT_TYPE_PITCH,
    LT_TYPE_HITFILTER,
    LT_NUM_TYPES
};

const char* ltTypeName(LTType type);

struct LTObject {
    int lua_wrap; // Reference to Lua wrapper table.
    LTType type;

    LTObject(LTType type);
    virtual ~LTObject();

    // For tweening and modification from lua.
    virtual LTfloat* field_ptr(const char *field_name);

    // Is this object of a certain type?
    bool hasType(LTType t);

    const char* typeName();
};

#endif
