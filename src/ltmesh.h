LT_INIT_DECL(ltmesh);

struct LTMesh : LTSceneNode {
    int dimensions;
    bool has_colors;
    bool has_normals;
    bool has_texture_coords;
    LTTexturedNode *texture;
    int texture_ref;
    LTDrawMode draw_mode;
    void *data;
    int stride;
    int size; // number of vertices

    LTvertbuf vertbuf;
    bool vb_dirty;

    LTfloat left, right, bottom, top, farz, nearz; // bounding box
    bool bb_dirty;

    LTMesh() { ltAbort(); };

    LTMesh(LTMesh *mesh); // clone
    LTMesh(LTTexturedNode *img);
    // dat should be malloc'd.  it will be freed by ~LTMesh.
    LTMesh(int dims, bool has_col, bool has_norm, bool has_tex_coords, LTImage *tex, LTDrawMode mode, void* dat, int sz);
    virtual ~LTMesh();

    virtual void draw();

    void stretch(
        /* about this point: */ LTfloat px, LTfloat py, LTfloat pz,
        /* this much along each axis: */ LTfloat left, LTfloat right, LTfloat down, LTfloat up, LTfloat backward, LTfloat forward);

    void shift(LTfloat sx, LTfloat sy, LTfloat sz);
    void merge(LTMesh *mesh);
    void grid(int rows, int columns);

    void compute_stride();
    void ensure_vb_uptodate();
    void ensure_bb_uptodate();
    void print();
};

void *lt_alloc_LTMesh(lua_State *L);
LTMesh *lt_expect_LTMesh(lua_State *L, int arg);
bool lt_is_LTMesh(lua_State *L, int arg);
