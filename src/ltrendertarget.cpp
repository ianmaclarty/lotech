#include "ltrendertarget.h"
#include "ltutil.h"

#define GL_DEPTH_COMPONENT16_EXT GL_DEPTH_COMPONENT16

LTRenderTarget::LTRenderTarget(int w, int h, 
        LTfloat vp_x1, LTfloat vp_y1, LTfloat vp_x2, LTfloat vp_y2,
        bool depthbuf, LTTextureFilter minfilter, LTTextureFilter magfilter)
        : LTSceneNode(LT_TYPE_RENDERTARGET) {
    LTRenderTarget::width = w;
    LTRenderTarget::height = h;
    LTRenderTarget::depthbuf_enabled = depthbuf;
    LTRenderTarget::vp_x1 = vp_x1;
    LTRenderTarget::vp_y1 = vp_y1;
    LTRenderTarget::vp_x2 = vp_x2;
    LTRenderTarget::vp_y2 = vp_y2;

    // Generate frame buffer.
    FBEXT(glGenFramebuffers)(1, &fbo);
    FBEXT(glBindFramebuffer)(FB_EXT(GL_FRAMEBUFFER), fbo);

    // Compute dimensions of target texture (must be powers of 2).
    tex_width = 64;
    tex_height = 64;
    while (tex_width < width) tex_width <<= 1;
    while (tex_height < height) tex_height <<= 1;

    // Generate texture.
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, lt2glFilter(minfilter)); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, lt2glFilter(magfilter));
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    int texsize = tex_width * tex_height * 4;
    void* texdata = malloc(texsize);
    #ifdef LTGLES1
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    #else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    #endif

    // Attach texture to frame buffer.
    FBEXT(glFramebufferTexture2D)(FB_EXT(GL_FRAMEBUFFER), FB_EXT(GL_COLOR_ATTACHMENT0), GL_TEXTURE_2D, texture_id, 0);
    GLenum status = FBEXT(glCheckFramebufferStatus)(FB_EXT(GL_FRAMEBUFFER));
    if (status != FB_EXT(GL_FRAMEBUFFER_COMPLETE)) {
        ltLog("Unable to create frame buffer of size %dx%d", tex_width, tex_height);
        ltAbort();
    }

    // Generate depth buffer if required.
    if (depthbuf_enabled) {
        FBEXT(glGenRenderbuffers)(1, &depth_renderbuf);
        FBEXT(glBindRenderbuffer)(FB_EXT(GL_RENDERBUFFER), depth_renderbuf);
        FBEXT(glRenderbufferStorage)(FB_EXT(GL_RENDERBUFFER), FB_EXT(GL_DEPTH_COMPONENT16), tex_width, tex_height);
        FBEXT(glFramebufferRenderbuffer)(FB_EXT(GL_FRAMEBUFFER), FB_EXT(GL_DEPTH_ATTACHMENT), FB_EXT(GL_RENDERBUFFER), depth_renderbuf); 
        FBEXT(glBindRenderbuffer)(FB_EXT(GL_RENDERBUFFER), 0);
    } else {
        depth_renderbuf = 0;
    }

    FBEXT(glBindFramebuffer)(FB_EXT(GL_FRAMEBUFFER), 0); 

    // Restore previously bound texture.
    LTtexid currtex = ltGetCurrentBoundTexture();
    if (currtex != 0) {
        glBindTexture(GL_TEXTURE_2D, currtex);
    }

    // Set up texture coords for drawing.
    int texel_w = LT_MAX_TEX_COORD / tex_width;
    int texel_h = LT_MAX_TEX_COORD / tex_height;
    LTtexcoord tex_right = width * texel_w;
    LTtexcoord tex_top = height * texel_h;
    tex_coords[0] = 0;          tex_coords[1] = tex_top;
    tex_coords[2] = tex_right;  tex_coords[3] = tex_top;
    tex_coords[4] = tex_right;  tex_coords[5] = 0;
    tex_coords[6] = 0;          tex_coords[7] = 0;
    glGenBuffers(1, &texbuf);
    glBindBuffer(GL_ARRAY_BUFFER, texbuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LTtexcoord) * 8, tex_coords, GL_STATIC_DRAW);

    // Set up world vertices for drawing.
    LTfloat pix_w = ltGetPixelWidth();
    LTfloat pix_h = ltGetPixelHeight();
    LTfloat world_width = (LTfloat)width * pix_w;
    LTfloat world_height = (LTfloat)height * pix_h;
    LTfloat world_left = - world_width * 0.5f;
    LTfloat world_bottom = - world_height * 0.5f;
    LTfloat world_top = world_height * 0.5f;
    LTfloat world_right = world_width * 0.5f;
    world_vertices[0] = world_left;
    world_vertices[1] = world_top;
    world_vertices[2] = world_right;
    world_vertices[3] = world_top;
    world_vertices[4] = world_right;
    world_vertices[5] = world_bottom;
    world_vertices[6] = world_left;
    world_vertices[7] = world_bottom;
    glGenBuffers(1, &vertbuf);
    glBindBuffer(GL_ARRAY_BUFFER, vertbuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, world_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    FBEXT(glBindFramebuffer)(FB_EXT(GL_FRAMEBUFFER), 0);
    LT_TRACE;
}

LTRenderTarget::~LTRenderTarget() {
    LT_TRACE;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    FBEXT(glBindFramebuffer)(FB_EXT(GL_FRAMEBUFFER), 0);

    LT_TRACE;
    glDeleteBuffers(1, &texbuf);
    LT_TRACE;
    glDeleteBuffers(1, &vertbuf);
    LT_TRACE;
    glDeleteTextures(1, &texture_id);
    LT_TRACE;
    if (depthbuf_enabled) {
        FBEXT(glDeleteRenderbuffers)(1, &depth_renderbuf);
    }
    LT_TRACE;
    FBEXT(glDeleteFramebuffers)(1, &fbo);
    LT_TRACE;
}

void LTRenderTarget::renderNode(LTSceneNode *node, LTColor *clear_color) {
    LT_TRACE;
    FBEXT(glBindFramebuffer)(FB_EXT(GL_FRAMEBUFFER), fbo);
    LT_TRACE;

    ltPrepareForRendering(
        0, 0, width, height, vp_x1, vp_y1, vp_x2, vp_y2,
        clear_color, depthbuf_enabled);
    LT_TRACE;

    node->draw();

    LT_TRACE;
    ltFinishRendering();
    LT_TRACE;
}

void LTRenderTarget::draw() {
    LT_TRACE;
    ltEnableTexture(texture_id);
    LT_TRACE;
    glBindBuffer(GL_ARRAY_BUFFER, vertbuf);
    LT_TRACE;
    glVertexPointer(2, GL_FLOAT, 0, 0);
    LT_TRACE;
    glBindBuffer(GL_ARRAY_BUFFER, texbuf);
    LT_TRACE;
    glTexCoordPointer(2, GL_SHORT, 0, 0);
    LT_TRACE;
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    LT_TRACE;
}
