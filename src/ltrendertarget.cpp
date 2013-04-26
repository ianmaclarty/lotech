/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltrendertarget)

struct LTRenderTargetAction : LTAction {
    LTRenderTarget *rt;
    LTRenderTargetAction(LTSceneNode *node) : LTAction(node) {
        rt = (LTRenderTarget*)node;
    };
    virtual bool doAction(LTfloat dt) {
        if (rt->child != NULL) {
            LTColor clear_color(0, 0, 0, 0);
            rt->renderNode(rt->child, &clear_color);
        }
        return false;
    }
};

LTRenderTarget::LTRenderTarget() {
    minfilter = LT_TEXTURE_FILTER_LINEAR;
    magfilter = LT_TEXTURE_FILTER_LINEAR;
    depthbuf_enabled = false;
    initialized = false;
    add_action(new LTRenderTargetAction(this));
}

void LTRenderTarget::init(lua_State *L) {
    LTTexturedNode::init(L);
    if (vp_x1 == 0.0f && vp_x2 == 0.0f) {
        vp_x1 = -1.0f;
        vp_x2 = 1.0f;
    }
    if (vp_y1 == 0.0f && vp_y2 == 0.0f) {
        vp_y1 = -1.0f;
        vp_y2 = 1.0f;
    }
    if (wld_x1 == 0.0f && wld_x2 == 0.0f) {
        LTfloat pix_w = ltGetPixelWidth();
        LTfloat world_width = (LTfloat)width * pix_w;
        wld_x1 = - world_width * 0.5f;
        wld_x2 = world_width * 0.5f;
    }
    if (wld_y1 == 0.0f && wld_y2 == 0.0f) {
        LTfloat pix_h = ltGetPixelHeight();
        LTfloat world_height = (LTfloat)height * pix_h;
        wld_y1 = - world_height * 0.5f;
        wld_y2 = world_height * 0.5f;
    }

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

    initialized = true;
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
    target->tex_coords[0] = 0;          target->tex_coords[1] = 0;
    target->tex_coords[2] = tex_right;  target->tex_coords[3] = 0;
    target->tex_coords[4] = tex_right;  target->tex_coords[5] = tex_top;
    target->tex_coords[6] = 0;          target->tex_coords[7] = tex_top;
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
    world_vertices[0] = wld_x1;  world_vertices[1] = wld_y1;
    world_vertices[2] = wld_x2;  world_vertices[3] = wld_y1;
    world_vertices[4] = wld_x2;  world_vertices[5] = wld_y2;
    world_vertices[6] = wld_x1;  world_vertices[7] = wld_y2;
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
    if (target->initialized) {
        setup_texture_coords(target);
    }
}

static void set_pheight(LTObject *obj, LTint val) {
    LTRenderTarget *target = (LTRenderTarget*)obj;
    target->height = val;
    if (target->initialized) {
        setup_texture_coords(target);
    }
}

void LTRenderTarget::visit_children(LTSceneNodeVisitor *v, bool reverse) {
    if (child != NULL) {
        v->visit(child);
    }
}

static LTObject *get_child(LTObject *obj) {
    return ((LTRenderTarget*)obj)->child;
}

static void set_child(LTObject *obj, LTObject *val) {
    LTRenderTarget *rt = (LTRenderTarget*)obj;
    LTSceneNode *old_child =rt->child;
    LTSceneNode *new_child = (LTSceneNode*)val;
    if (old_child != NULL) {
        old_child->exit(rt);
    }
    rt->child = new_child;
    if (new_child != NULL) {
        new_child->enter(rt);
    }
}

LT_REGISTER_TYPE(LTRenderTarget, "lt.RenderTarget", "lt.TexturedNode")
LT_REGISTER_PROPERTY_OBJ(LTRenderTarget, child, LTSceneNode, get_child, set_child);
LT_REGISTER_PROPERTY_INT(LTRenderTarget, pwidth, &get_pwidth, &set_pwidth);
LT_REGISTER_PROPERTY_INT(LTRenderTarget, pheight, &get_pheight, &set_pheight);
LT_REGISTER_FIELD_FLOAT(LTRenderTarget, vp_x1)
LT_REGISTER_FIELD_FLOAT(LTRenderTarget, vp_y1)
LT_REGISTER_FIELD_FLOAT(LTRenderTarget, vp_x2)
LT_REGISTER_FIELD_FLOAT(LTRenderTarget, vp_y2)
LT_REGISTER_FIELD_FLOAT(LTRenderTarget, wld_x1)
LT_REGISTER_FIELD_FLOAT(LTRenderTarget, wld_y1)
LT_REGISTER_FIELD_FLOAT(LTRenderTarget, wld_x2)
LT_REGISTER_FIELD_FLOAT(LTRenderTarget, wld_y2)
static const LTEnumConstant RenderTarget_filter_enum_vals[] = {
    {"linear", LT_TEXTURE_FILTER_LINEAR},
    {"nearest", LT_TEXTURE_FILTER_NEAREST},
    {NULL, 0}};
LT_REGISTER_FIELD_ENUM(LTRenderTarget, minfilter, LTTextureFilter, RenderTarget_filter_enum_vals)
LT_REGISTER_FIELD_ENUM(LTRenderTarget, magfilter, LTTextureFilter, RenderTarget_filter_enum_vals)
