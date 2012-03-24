/* Copyright (C) 2012 Ian MacLarty */
#ifndef LTRENDERTARGET_H
#define LTRENDERTARGET_H

#include "ltcommon.h"
#include "ltscene.h"
#include "ltimage.h"
#include "ltgraphics.h"

struct LTRenderTarget : LTSceneNode {
    LTframebuf      fbo;
    LTtexid         texture_id;
    bool            depthbuf_enabled;

    int             width;
    int             height;

    // The target texture dimensions must be powers of 2, so may
    // be different from the specified width and height.
    // These are also used for the dimensions of the depth buffer.
    int             tex_width;
    int             tex_height;

    // For drawing the render target.
    LTfloat         world_vertices[8];
    LTvertbuf       vertbuf; // for world vertices
    LTtexcoord      tex_coords[8];
    LTvertbuf       texbuf;

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

    LTRenderTarget(int w, int h,
        LTfloat vp_x1, LTfloat vp_y1, LTfloat vp_x2, LTfloat vp_y2,
        LTfloat wld_x1, LTfloat wld_y1, LTfloat wld_x2, LTfloat wld_y2,
        bool depthbuf, LTTextureFilter minfilter, LTTextureFilter magfilter);
    virtual ~LTRenderTarget();

    // Render a scene node to this rendertarget, optionally clearing to
    // the given color first (clear_color may be NULL).
    void renderNode(LTSceneNode *node, LTColor *clear_color);

    virtual void draw();
    virtual void preContextChange();
    virtual void postContextChange();

private:

    void setup();
};

#endif
