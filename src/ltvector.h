/* Copyright (C) 2010 Ian MacLarty */

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

struct LTDrawVector : LTSceneNode {
    LTVector *vector;
    int dimensions;
    int vertex_offset;
    int color_offset;
    int texture_offset;
    LTDrawMode mode;
    LTTexturedNode *image;

    LTDrawVector(LTDrawMode mode, LTVector *vector, int dims, int vertex_os, int color_os, int tex_os, LTTexturedNode *image);
    virtual void draw();
};

struct LTDrawTexturedQuads : LTSceneNode {
    LTVector *vector;
    int offset;
    unsigned short *elements;
    unsigned short num_elems;
    LTTexturedNode *image;

    LTDrawTexturedQuads(LTVector *vector, int offset, LTTexturedNode *image);
    virtual ~LTDrawTexturedQuads();
    virtual void draw();
};
