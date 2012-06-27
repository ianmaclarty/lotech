LT_INIT_DECL(ltmesh);

struct LTMesh : LTSceneNode {
    int dimensions;
    bool has_color;
    bool has_normals;
    LTImage *texture; // NULL if no texture
    LTDrawMode draw_mode;

    void *data;
    int stride;
    int size; // number of vertices

    LTvertbuf vertbuf;

    bool dirty;

    LTMesh() { ltAbort(); };
    // dat should be malloc'd
    LTMesh(int dims, bool has_col, bool has_norm, LTImage *tex, LTDrawMode mode, void* dat, int sz);
    virtual ~LTMesh();
    virtual void draw();

    void setup();
    void ensure_buffer_uptodate();
};

void *lt_alloc_LTMesh(lua_State *L);
