#include "lt.h"

LTRenderTarget::LTRenderTarget(int w, int h, 
        LTfloat vp_x1, LTfloat vp_y1, LTfloat vp_x2, LTfloat vp_y2,
        LTfloat wld_x1, LTfloat wld_y1, LTfloat wld_x2, LTfloat wld_y2,
        bool depthbuf, LTTextureFilter minfilter, LTTextureFilter magfilter)
        : LTTexturedNode(LT_TYPE_RENDERTARGET) {
    LTRenderTarget::width = w;
    LTRenderTarget::height = h;
    LTRenderTarget::depthbuf_enabled = depthbuf;
    LTRenderTarget::vp_x1 = vp_x1;
    LTRenderTarget::vp_y1 = vp_y1;
    LTRenderTarget::vp_x2 = vp_x2;
    LTRenderTarget::vp_y2 = vp_y2;
    LTRenderTarget::wld_x1 = wld_x1;
    LTRenderTarget::wld_y1 = wld_y1;
    LTRenderTarget::wld_x2 = wld_x2;
    LTRenderTarget::wld_y2 = wld_y2;

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

    setup();
}

LTRenderTarget::~LTRenderTarget() {
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

void LTRenderTarget::preContextChange() {
    ltDeleteVertBuffer(texbuf);
    ltDeleteVertBuffer(vertbuf);
    ltDeleteFramebuffer(fbo);
}

void LTRenderTarget::postContextChange() {
    setup();
}

static void setup_texture_coords(LTRenderTarget *target) {
    // Set up texture coords for drawing.
    int texel_w = LT_MAX_TEX_COORD / target->tex_width;
    int texel_h = LT_MAX_TEX_COORD / target->tex_height;
    LTtexcoord tex_right = target->width * texel_w;
    LTtexcoord tex_top = target->height * texel_h;
    target->tex_coords[0] = 0;          target->tex_coords[1] = tex_top;
    target->tex_coords[2] = tex_right;  target->tex_coords[3] = tex_top;
    target->tex_coords[4] = tex_right;  target->tex_coords[5] = 0;
    target->tex_coords[6] = 0;          target->tex_coords[7] = 0;
    ltBindVertBuffer(target->texbuf);
    ltStaticVertBufferData(sizeof(LTtexcoord) * 8, target->tex_coords);
}

void LTRenderTarget::setup() {
    // Generate frame buffer.
    fbo = ltGenFramebuffer();
    ltBindFramebuffer(fbo);

    // Attach texture to frame buffer.
    ltFramebufferTexture(texture_id);
    if (!ltFramebufferComplete()) {
        ltLog("Unable to create frame buffer of size %dx%d", tex_width, tex_height);
        ltAbort();
    }

    texbuf = ltGenVertBuffer();
    setup_texture_coords(this);

    // Set up world vertices for drawing.
    world_vertices[0] = wld_x1;  world_vertices[1] = wld_y2;
    world_vertices[2] = wld_x2;  world_vertices[3] = wld_y2;
    world_vertices[4] = wld_x2;  world_vertices[5] = wld_y1;
    world_vertices[6] = wld_x1;  world_vertices[7] = wld_y1;
    vertbuf = ltGenVertBuffer();
    ltBindVertBuffer(vertbuf);
    ltStaticVertBufferData(sizeof(LTfloat) * 8, world_vertices);
}

static LTint get_pwidth(LTObject *obj) {
    return ((LTRenderTarget*)obj)->width;
}

static LTint get_pheight(LTObject *obj) {
    return ((LTRenderTarget*)obj)->height;
}

static void set_pwidth(LTObject *obj, LTint val) {
    LTRenderTarget *target = (LTRenderTarget*)obj;
    target->width = val;
    setup_texture_coords(target);
}

static void set_pheight(LTObject *obj, LTint val) {
    LTRenderTarget *target = (LTRenderTarget*)obj;
    target->height = val;
    setup_texture_coords(target);
}

LTFieldDescriptor* LTRenderTarget::fields() {
    static LTFieldDescriptor flds[] = {
        {"pwidth",  LT_FIELD_TYPE_INT, -1, (void*)get_pwidth, (void*)set_pwidth, LT_ACCESS_FULL},
        {"pheight", LT_FIELD_TYPE_INT, -1, (void*)get_pheight, (void*)set_pheight, LT_ACCESS_FULL},
        LT_END_FIELD_DESCRIPTOR_LIST
    };
    return flds;
}

