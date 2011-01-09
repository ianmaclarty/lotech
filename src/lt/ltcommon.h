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

struct LTObject {
    int ref_count;

    LTObject() {
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
};

#endif
