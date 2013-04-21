/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltvector)

struct LTVector : LTObject {
    int stride;   // Size of each record (in 4 byte words)
    int capacity; // In terms of records.
    int size;     // How many records are used.

    LTfloat *data;

    LTVector() { ltAbort(); };
    LTVector(int capacity, int stride);
    virtual ~LTVector();
};

//struct LTEmitterColumnSpec {
//    LTfloat value;
//    LTfloat variance;
//};
//
//struct LTEmitter : LTObject {
//    LTVector *vector;
//    LTfloat rate;
//    int time_to_live_col;
//    LTEmitterColumnSpec *col_vals;
//
//    void advance(LTfloat dt);
//};

struct LTDrawVector : LTSceneNode {
    LTVector *vector;
    int dimensions;
    int vertex_offset;
    int color_offset;
    int texture_offset;
    LTDrawMode mode;
    LTTexturedNode *image;

    LTDrawVector();
    virtual void init(lua_State *L);
    virtual void draw();
};

//struct LTDrawTexturedQuads : LTSceneNode {
//    LTVector *vector;
//    int offset;
//    unsigned short *elements;
//    unsigned short num_elems;
//    LTTexturedNode *image;
//
//    virtual ~LTDrawTexturedQuads();
//    virtual void init(lua_State *L);
//    virtual void draw();
//};


void *lt_alloc_LTVector(lua_State *L);
LTVector *lt_expect_LTVector(lua_State *L, int arg);
