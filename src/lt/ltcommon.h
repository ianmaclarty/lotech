/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTCOMMON_H
#define LTCOMMON_H

#include <stdlib.h>
#include <stdio.h>

#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
#define ct_assert(e) enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }

typedef float           LTfloat;
typedef unsigned int    LTuint;
typedef float           LTsecs;
typedef float           LTdegrees;
typedef unsigned int    LTuint32;
ct_assert(sizeof(LTuint32) == 4);

void ltAbort(const char *fmt, ...);
void ltLog(const char *fmt, ...);

// Used for reflection.  Any subclasses of LTObject that can be used
// in Lua code should have an entry in this enumeration.
enum LTType {
    LT_TYPE_OBJECT,
    LT_TYPE_PROP,
    LT_TYPE_TRANSLATOR,
    LT_TYPE_ROTATOR,
    LT_TYPE_SCALOR,
    LT_TYPE_TINTER,
    LT_TYPE_SCENE,
    LT_TYPE_IMAGE,
    LT_TYPE_ATLAS,
    LT_NUM_TYPES
};

// Gives the supertype of each type.
static const LTType supertypes[] = {
    LT_TYPE_OBJECT, // LT_TYPE_OBJECT
    LT_TYPE_OBJECT, // LT_TYPE_PROP
    LT_TYPE_PROP,   // LT_TYPE_TRANSLATOR
    LT_TYPE_PROP,   // LT_TYPE_ROTATOR
    LT_TYPE_PROP,   // LT_TYPE_SCALOR
    LT_TYPE_PROP,   // LT_TYPE_TINTER
    LT_TYPE_PROP,   // LT_TYPE_SCENE
    LT_TYPE_PROP,   // LT_TYPE_IMAGE
    LT_TYPE_OBJECT, // LT_TYPE_ATLAS
};

ct_assert(sizeof(supertypes) == (int)LT_NUM_TYPES * 4);

struct LTObject {
    LTType type;
    int ref_count;

    LTObject(LTType type) {
        LTObject::type = type;
        ref_count = 0;
    }
    virtual ~LTObject() {}

    void retain() {
        ref_count++;
    }

    void release() {
        ref_count--;
        if (ref_count <= 0) {
            delete this;
        }
    }

    // For tweening.
    virtual LTfloat* field_ptr(const char *field_name) {
        return NULL;
    }

    // Is this object of a certain type?
    bool hasType(LTType t) {
        if (t == LT_TYPE_OBJECT) {
            return true;
        }
        LTType t1 = type;
        while (t1 != LT_TYPE_OBJECT) {
            if (t == t1) {
                return true;
            }
            t1 = supertypes[t1];
        }
        return false;
    }
};

#endif
