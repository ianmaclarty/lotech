/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltrendertarget)

struct LTRenderTarget : LTTexturedNode {
    LTframebuf      fbo;
    bool            depthbuf_enabled;

    LTint             width;
    LTint             height;

    // The target texture dimensions must be powers of 2, so may
    // be different from the specified width and height.
    // These are also used for the dimensions of the depth buffer.
    LTint             tex_width;
    LTint             tex_height;

    // Viewport
    LTfloat vp_x1;
    LTfloat vp_y1;
    LTfloat vp_x2;
    LTfloat vp_y2;

    // World coords
    LTfloat wld_x1;
    LTfloat wld_y1;
    LTfloat wld_x2;
    LTfloat wld_y2;

    LTTextureFilter minfilter;
    LTTextureFilter magfilter;

    bool initialized;

    LTSceneNode *child;

    LTRenderTarget();
    virtual ~LTRenderTarget();

    virtual void init(lua_State *L);
    virtual void visit_children(LTSceneNodeVisitor *v, bool reverse);

    // Render a scene node to this rendertarget, optionally clearing to
    // the given color first (clear_color may be NULL).
    void renderNode(LTSceneNode *node, LTColor *clear_color);

    virtual void preContextChange();
    virtual void postContextChange();

private:

    void setup();
};

LTRenderTarget *lt_expect_LTRenderTarget(lua_State *L, int arg);
