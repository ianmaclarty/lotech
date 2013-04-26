/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltmesh);

struct LTVertData {
    LTVec3 xyz;
    LTColor color;
    LTVec3 normal;
    LTTexCoord uv;
};

struct LTMesh : LTSceneNode {
    int dimensions;
    bool has_colors;
    bool has_normals;
    bool has_texture_coords;
    LTTexturedNode *texture;
    int texture_ref;
    LTDrawMode draw_mode;
    LTVertData *vdata;
    int size; // number of vertices
    LTvertbuf vertbuf;
    bool vb_dirty;
    LTfloat left, right, bottom, top, farz, nearz; // bounding box
    bool bb_dirty;
    LTvertindex *indices;
    int num_indices;
    bool indices_dirty;

    LTMesh();
    LTMesh(LTMesh *mesh); // clone
    LTMesh(LTTexturedNode *img);
    // dat should be allocated with new[].  it will be deleted by ~LTMesh.
    LTMesh(int dims, bool has_col, bool has_norm, bool has_tex_coords,
        LTImage *tex, LTDrawMode mode, LTVertData *dat, int sz);
    virtual ~LTMesh();

    virtual void draw();

    void stretch(
        /* about this point: */ LTfloat px, LTfloat py, LTfloat pz,
        /* this much along each axis: */ LTfloat left, LTfloat right, LTfloat down,
                                         LTfloat up, LTfloat backward, LTfloat forward);

    void shift(LTfloat sx, LTfloat sy, LTfloat sz);
    void merge(LTMesh *mesh);
    void grid(int rows, int columns);

    void ensure_vb_uptodate();
    void ensure_bb_uptodate();
    void resize_data(int sz);
    void resize_indices(int sz);
    void compute_normals();
    void print();

private:
    void *generate_vbo_data();
};

void *lt_alloc_LTMesh(lua_State *L);
LTMesh *lt_expect_LTMesh(lua_State *L, int arg);
bool lt_is_LTMesh(lua_State *L, int arg);
