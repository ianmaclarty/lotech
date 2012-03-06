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
    LTrenderbuf     depth_renderbuf;
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
    LTtexbuf        texbuf;

    LTRenderTarget(int w, int h, bool depthbuf, LTTextureFilter minfilter, LTTextureFilter magfilter);
    virtual ~LTRenderTarget();

    // Render a scene node to this rendertarget, optionally clearing to
    // the given color first (clear_color may be NULL).
    void renderNode(LTSceneNode *node, LTColor *clear_color);

    virtual void draw();
};

#endif
