/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTVECTOR_H
#define LTVECTOR_H

#include "ltcommon.h"
#include "ltimage.h"

struct LTVector : LTObject {
    int stride;   // Size of each record
    int capacity; // In terms of records.
    int size;     // How many records are used.

    LTfloat *data;

    LTVector(int capacity, int stride);
    virtual ~LTVector();
};

struct LTEmitterColumnSpec {
    LTfloat value;
    LTfloat variance;
};

struct LTEmitter : LTObject {
    LTVector *vector;
    LTfloat rate;
    int time_to_live_col;
    LTEmitterColumnSpec *col_vals;

    void advance(LTfloat dt);
};

enum LTDrawMode {
    LT_DRAWMODE_TRIANGLES      = GL_TRIANGLES,
    LT_DRAWMODE_TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
    LT_DRAWMODE_TRIANGLE_FAN   = GL_TRIANGLE_FAN,
    LT_DRAWMODE_POINTS         = GL_POINTS,
};

struct LTDrawVector : LTSceneNode {
    LTVector *vector;
    int dimensions;
    int vertex_offset;
    int color_offset;
    int texture_offset;
    LTDrawMode mode;
    LTImage *image;

    LTDrawVector(LTDrawMode mode, LTVector *vector, int dims, int vertex_os, int color_os, int tex_os, LTImage *image);
    virtual void draw();
};

struct LTDrawTexturedQuads : LTSceneNode {
    LTVector *vector;
    int offset;
    unsigned short *elements;
    unsigned short num_elems;
    LTImage *image;

    LTDrawTexturedQuads(LTVector *vector, int offset, LTImage *image);
    virtual ~LTDrawTexturedQuads();
    virtual void draw();
};

#endif
