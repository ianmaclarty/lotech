#include "ltrendertarget.h"
#include "ltutil.h"
#include "ltopengl.h"

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
    fbo = ltGenFramebuffer();
    ltBindFramebuffer(fbo);

    // Compute dimensions of target texture (must be powers of 2).
    tex_width = 64;
    tex_height = 64;
    while (tex_width < width) tex_width <<= 1;
    while (tex_height < height) tex_height <<= 1;

    // Generate texture.
    texture_id = ltGenTexture();
    ltBindTexture(texture_id);
    ltTextureMinFilter(minfilter);
    ltTextureMagFilter(magfilter);
    ltTexImage(tex_width, tex_height, NULL);

    // Attach texture to frame buffer.
    ltFramebufferTexture(texture_id);
    if (!ltFramebufferComplete()) {
        ltLog("Unable to create frame buffer of size %dx%d", tex_width, tex_height);
        ltAbort();
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
    texbuf = ltGenVertBuffer();
    ltBindVertBuffer(texbuf);
    ltStaticVertBufferData(sizeof(LTtexcoord) * 8, tex_coords);

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
    vertbuf = ltGenVertBuffer();
    ltBindVertBuffer(vertbuf);
    ltStaticVertBufferData(sizeof(LTfloat) * 8, world_vertices);
}

LTRenderTarget::~LTRenderTarget() {
    ltDeleteVertBuffer(texbuf);
    ltDeleteVertBuffer(vertbuf);
    ltDeleteFramebuffer(fbo);
    ltDeleteTexture(texture_id);
}

void LTRenderTarget::renderNode(LTSceneNode *node, LTColor *clear_color) {
    ltBindFramebuffer(fbo);

    ltPrepareForRendering(
        0, 0, width, height, vp_x1, vp_y1, vp_x2, vp_y2,
        clear_color, depthbuf_enabled);

    node->draw();

    ltFinishRendering();
}

void LTRenderTarget::draw() {
    ltEnableTexture(texture_id);
    ltBindVertBuffer(vertbuf);
    ltVertexPointer(2, LT_VERT_DATA_TYPE_FLOAT, 0, 0);
    ltBindVertBuffer(texbuf);
    ltTexCoordPointer(2, LT_VERT_DATA_TYPE_SHORT, 0, 0);
    ltDrawArrays(LT_DRAWMODE_TRIANGLE_FAN, 0, 4);
}
