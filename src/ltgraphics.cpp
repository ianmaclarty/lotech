/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltgraphics)

static LTframebuf main_framebuffer = 0;

// Actual screen dimensions in pixels.
static int screen_width = 480;
static int screen_height = 320;

// Position of the glViewport (in pixels).
static int screen_viewport_x = 0;
static int screen_viewport_y = 0;
static int screen_viewport_width = 480;
static int screen_viewport_height = 320;

// Screen dimensions used to compute size of pixels in loaded
// images and the aspect ration when using an envelope.
// These don't have to match the actual screen dimensions.
static LTfloat design_width = 960.0f;
static LTfloat design_height = 640.0f;

// User space dimensions.
static LTfloat viewport_left = -1.0f;
static LTfloat viewport_bottom = -1.0f;
static LTfloat viewport_right = 1.0f;
static LTfloat viewport_top = 1.0f;
static LTfloat viewport_width = 2.0f;
static LTfloat viewport_height = 2.0f;

// Used for recording the main viewport when rendering into
// a offscreen render target.
static LTfloat orig_viewport_left = -1.0f;
static LTfloat orig_viewport_bottom = -1.0f;
static LTfloat orig_viewport_right = 1.0f;
static LTfloat orig_viewport_top = 1.0f;
static LTfloat orig_viewport_width = 2.0f;
static LTfloat orig_viewport_height = 2.0f;

static LTfloat design_viewport_left = -1.0f;
static LTfloat design_viewport_bottom = -1.0f;
static LTfloat design_viewport_right = 1.0f;
static LTfloat design_viewport_top = 1.0f;
static LTfloat design_viewport_width = 2.0f;
static LTfloat design_viewport_height = 2.0f;

// Dimensions of a (design) pixel in user space.
// Used to decide how to scale images by default.
static LTfloat pixel_width = viewport_width / design_width;
static LTfloat pixel_height = viewport_height / design_height;

static LTDisplayOrientation display_orientation = LT_DISPLAY_ORIENTATION_LANDSCAPE;

static std::list<LTColor> tint_stack;
static std::list<LTBlendMode> blend_mode_stack;
static std::list<LTTextureMode> texture_mode_stack;

void ltInitGraphics() {
    static LTColor clear_color(0.0f, 0.0f, 0.0f, 0.0f);
    #ifdef LTDEPTHBUF
    static bool clear_depthbuf = true;
    #else
    static bool clear_depthbuf = false;
    #endif

    ltBindFramebuffer(main_framebuffer);

    ltPrepareForRendering(
        screen_viewport_x, screen_viewport_y,
        screen_viewport_width, screen_viewport_height,
        viewport_left, viewport_bottom,
        viewport_right, viewport_top,
        &clear_color, clear_depthbuf);
}

void ltPrepareForRendering(
    int screen_viewport_x, int screen_viewport_y,
    int screen_viewport_width, int screen_viewport_height,
    LTfloat vp_left, LTfloat vp_bottom,
    LTfloat vp_right, LTfloat vp_top,
    LTColor *clear_color, bool clear_depthbuf) 
{
    // Record original viewport values.
    orig_viewport_left = viewport_left;
    orig_viewport_bottom = viewport_bottom;
    orig_viewport_right = viewport_right;
    orig_viewport_top = viewport_top;
    orig_viewport_width = viewport_width;
    orig_viewport_height = viewport_height;

    // Update viewport variables to new values
    viewport_left = vp_left;
    viewport_bottom = vp_bottom;
    viewport_right = vp_right;
    viewport_top = vp_top;
    viewport_width = vp_right - vp_left;
    viewport_height = vp_top - vp_bottom;

    ltDisableDither();
    ltDisableAlphaTest();
    ltDisableStencilTest();
    ltDisableFog();
    ltFogMode(LT_FOG_MODE_LINEAR);
    ltDisableTextures();
    ltDisableDepthTest();
    ltEnableDepthMask();
    ltDepthFunc(LT_DEPTH_FUNC_LEQUAL);
    if (clear_color != NULL || clear_depthbuf) {
        if (clear_color) {
            ltClearColor(clear_color->red, clear_color->green, clear_color->blue, clear_color->alpha);
        }
        ltClear(clear_color != NULL, clear_depthbuf);
    }
    ltColor(1.0f, 1.0f, 1.0f, 1.0f);
    tint_stack.clear();
    blend_mode_stack.clear();
    texture_mode_stack.clear();
    ltEnableVertexArrays();
    #ifndef LTGLES1
    //ltEnableIndexArrays();
    #endif
    ltBlendMode(LT_BLEND_MODE_NORMAL);
    ltTextureMode(LT_TEXTURE_MODE_MODULATE);
    ltMatrixMode(LT_MATRIX_MODE_PROJECTION);
    ltLoadIdentity();
    ltViewport(screen_viewport_x, screen_viewport_y, screen_viewport_width, screen_viewport_height);
    ltOrtho(viewport_left, viewport_right, viewport_bottom, viewport_top, -1.0f, 1.0f);
    ltMatrixMode(LT_MATRIX_MODE_TEXTURE);
    ltLoadIdentity();
    ltScale(1.0f / (LTfloat)LT_MAX_TEX_COORD, 1.0f / (LTfloat)LT_MAX_TEX_COORD, 1.0f);
    ltMatrixMode(LT_MATRIX_MODE_MODELVIEW);
    ltLoadIdentity();
}

void ltFinishRendering() {
    viewport_left   = orig_viewport_left;
    viewport_bottom = orig_viewport_bottom;
    viewport_right  = orig_viewport_right;
    viewport_top    = orig_viewport_top;
    viewport_width  = orig_viewport_width;
    viewport_height = orig_viewport_height;
}

void ltSetMainFrameBuffer(LTframebuf fbo) {
    main_framebuffer = fbo;
}

void ltSetViewPort(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2) {
    design_viewport_left = x1;
    design_viewport_right = x2;
    design_viewport_bottom = y1;
    design_viewport_top = y2;
    design_viewport_width = design_viewport_right - design_viewport_left;
    design_viewport_height = design_viewport_top - design_viewport_bottom;
    viewport_left = design_viewport_left;
    viewport_right = design_viewport_right;
    viewport_top = design_viewport_top;
    viewport_bottom = design_viewport_bottom;
    viewport_width = design_viewport_width;
    viewport_height = design_viewport_height;
    pixel_width = viewport_width / design_width;
    pixel_height = viewport_height / design_height;
}

#ifdef LTADS
static LTfloat get_ad_height_ratio() {
    #ifdef LTIOS
    LTfloat h;
    LTfloat d;
    if (ltIsIPad()) {
        if (display_orientation == LT_DISPLAY_ORIENTATION_PORTRAIT) {
            h = 1024.0f;
        } else {
            h = 768.0f;
        }
        d = 90.0f;
    } else {
        if (display_orientation == LT_DISPLAY_ORIENTATION_PORTRAIT) {
            h = 480.0f;
        } else {
            h = 320.0f;
        }
        d = 50.0f;
    }
    return d / h;
    #else
    return 0.0f;
    #endif
}
#endif // LTADS

void ltAdjustViewportAspectRatio() {
    LTfloat w0 = design_width;
    LTfloat h0 = design_height;
    LTfloat w1 = (LTfloat)screen_width;
    LTfloat h1 = (LTfloat)screen_height;
    if (lt_envelope) {
        LTfloat sy = h1 / h0;
        LTfloat dx = (w1 - w0 * sy) / (2.0f * w1);
        LTfloat sx = w1 / w0;
        LTfloat dy = (h1 - h0 * sx) / (2.0f * h1);
        if (dx > 0.01f) {
            screen_viewport_x = (int)(dx * (float)screen_width);
            screen_viewport_y = 0;
            screen_viewport_width = screen_width - (int)(dx * (float)screen_width * 2.0f);
            screen_viewport_height = screen_height;
        } else {
            screen_viewport_x = 0;
            screen_viewport_width = screen_width;
            if (dy > 0.01) {
                screen_viewport_y = (int)(dy * (float)screen_height);
                screen_viewport_height = screen_height - (int)(dy * (float)screen_height)*2;
            } else {
                screen_viewport_y = 0;
                screen_viewport_height = screen_height;
            }
        }
    } else {
        LTfloat sy = h1 / h0;
        LTfloat dx = (w1 - w0 * sy) / (2.0f * w0 * sy);
        LTfloat sx = w1 / w0;
        LTfloat dy = (h1 - h0 * sx) / (2.0f * h0 * sx);
        if (dx > 0.01f) {
            viewport_left = design_viewport_left - design_viewport_width * dx;
            viewport_right = design_viewport_right + design_viewport_width * dx;
            viewport_top = design_viewport_top;
            viewport_bottom = design_viewport_bottom;
        } else {
            viewport_left = design_viewport_left;
            viewport_right = design_viewport_right;
            if (dy > 0.01f) {
                viewport_bottom = design_viewport_bottom - design_viewport_height * dy;
                viewport_top = design_viewport_top + design_viewport_height * dy;
            } else {
                viewport_bottom = design_viewport_bottom;
                viewport_top = design_viewport_top;
            }
        }
        viewport_width = viewport_right - viewport_left;
        viewport_height = viewport_top - viewport_bottom;
    }

    // Make space for ads.
    #ifdef LTADS
    LTfloat r = get_ad_height_ratio();
    if (LTADS == LT_AD_TOP) {
        viewport_top += viewport_height * r;
    } else {
        viewport_bottom -= viewport_height * r;
    }
    viewport_height = viewport_top - viewport_bottom;
    #endif
}

void ltDrawConnectingOverlay() {
#ifdef LTDEVMODE
    static int s = 0;
    static int c = 0;
    LTfloat l = viewport_right - viewport_width * 0.1;
    LTfloat r = viewport_right;
    LTfloat b = viewport_top - viewport_height * 0.1;
    LTfloat t = viewport_top;
    if (ltClientIsTryingToConnect()) {
        ltLoadIdentity();
        if (s & 16) {
            ltPushTint(0.4f, 0.4f, 0.4f, 1.0f);
        } else {
            ltPushTint(0.8f, 0.8f, 0.8f, 1.0f);
        }
        ltDrawRect(l, b, r, t);
        ltPopTint();
        s++;
        c = 0;
    } else if (ltClientIsReady()) {
        ltLoadIdentity();
        // Connect successful.  Show green box.
        if (c < 60) {
            ltPushTint(0.1f, 0.8f, 0.1f, 1.0f);
            ltDrawRect(l, b, r, t);
            ltPopTint();
            c++;
        }
    } else {
        ltLoadIdentity();
        // Connect unsuccessful.  Show red box.
        if (c < 60) {
            ltPushTint(1.0f, 0.0f, 0.0f, 1.0f);
            ltDrawRect(l, b, r, t);
            ltPopTint();
            c++;
        }
    }
#endif
}

void ltDrawAdBackground() {
    #ifdef LTADS
    LTfloat h = get_ad_height_ratio() * viewport_height;
    LTfloat l = viewport_left;
    LTfloat r = viewport_right;
    LTfloat t, b;
    if (LTADS == LT_AD_TOP) {
        t = viewport_top;
        b = viewport_top - h;
    } else {
        t = viewport_bottom + h;
        b = viewport_bottom;
    }
    ltPushTint(0.2f, 0.2f, 0.2f, 1.0f);
    ltDrawRect(l, b, r, t);
    ltPopTint();
    #endif
}

void ltSetScreenSize(int width, int height) {
    screen_width = width;
    screen_height = height;
    screen_viewport_x = 0;
    screen_viewport_y = 0;
    screen_viewport_width = width;
    screen_viewport_height = height;
}

void ltSetDesignScreenSize(LTfloat width, LTfloat height) {
    design_width = width;
    design_height = height;
    pixel_width = viewport_width / design_width;
    pixel_height = viewport_height / design_height;
}

void ltGetDesignScreenSize(LTfloat *width, LTfloat *height) {
    *width = design_width;
    *height = design_height;
}

void ltSetDisplayOrientation(LTDisplayOrientation orientation) {
    display_orientation = orientation;
}

LTDisplayOrientation ltGetDisplayOrientation() {
    return display_orientation;
}

void ltResizeScreen(int width, int height) {
    ltSetScreenSize(width, height);
}

LTfloat ltGetPixelWidth() {
    return pixel_width;
}

LTfloat ltGetPixelHeight() {
    return pixel_height;
}

LTfloat ltGetViewPortX(LTfloat screen_x) {
#ifdef LTIOS
    static LTfloat scaling = 0.0f;
    if (scaling == 0.0f) {
        scaling = ltIOSScaling();
    }
    return viewport_left + (((screen_x - screen_viewport_x) * scaling) / (LTfloat)screen_viewport_width) * viewport_width;
#else
    return viewport_left + ((screen_x - screen_viewport_x) / (LTfloat)screen_viewport_width) * viewport_width;
#endif
}

LTfloat ltGetViewPortY(LTfloat screen_y) {
#ifdef LTIOS
    static LTfloat scaling = 0.0f;
    if (scaling == 0.0f) {
        scaling = ltIOSScaling();
    }
    return viewport_top - (((screen_y - screen_viewport_y) * scaling) / (LTfloat)screen_viewport_height) * viewport_height;
#else
    return viewport_top - ((screen_y - screen_viewport_y) / (LTfloat)screen_viewport_height) * viewport_height;
#endif
}

LTfloat ltGetViewPortLeftEdge() {
    return viewport_left;
}

LTfloat ltGetViewPortRightEdge() {
    return viewport_right;
}

LTfloat ltGetViewPortBottomEdge() {
    return viewport_bottom;
}

LTfloat ltGetViewPortTopEdge() {
    return viewport_top;
}

void ltPushPerspective(LTfloat nearz, LTfloat origin, LTfloat farz, LTfloat vanish_x, LTfloat vanish_y) {
    ltMatrixMode(LT_MATRIX_MODE_PROJECTION);
    ltPushMatrix();
    ltLoadIdentity();
    LTfloat r = (origin - nearz) / origin; 
    LTfloat near_half_width = 0.5f * (viewport_width - r * viewport_width);
    LTfloat near_half_height = 0.5f * (viewport_height - r * viewport_height);
    vanish_x *= (1.0f - r);
    vanish_y *= (1.0f - r);
    ltFrustum(-near_half_width - vanish_x, near_half_width - vanish_x,
        -near_half_height - vanish_y, near_half_height - vanish_y, nearz, farz);
    ltMatrixMode(LT_MATRIX_MODE_MODELVIEW);
    ltPushMatrix();
    ltTranslate(-(viewport_width * 0.5f + viewport_left),
        -(viewport_height * 0.5f + viewport_bottom), -origin);
}

void ltPopPerspective() {
    ltMatrixMode(LT_MATRIX_MODE_PROJECTION);
    ltPopMatrix();
    ltMatrixMode(LT_MATRIX_MODE_MODELVIEW);
    ltPopMatrix();
}

void ltPushTint(LTfloat r, LTfloat g, LTfloat b, LTfloat a) {
    LTColor new_top(r, g, b, a);
    if (!tint_stack.empty()) {
        LTColor *top = &tint_stack.front();
        new_top.red *= top->red;
        new_top.green *= top->green;
        new_top.blue *= top->blue;
        new_top.alpha *= top->alpha;
    }
    tint_stack.push_front(new_top);
    ltColor(new_top.red, new_top.green, new_top.blue, new_top.alpha);
}

void ltPopTint() {
    if (!tint_stack.empty()) {
        tint_stack.pop_front();
        if (!tint_stack.empty()) {
            LTColor *top = &tint_stack.front();
            ltColor(top->red, top->green, top->blue, top->alpha);
        } else {
            ltColor(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
}

void ltPeekTint(LTColor *color) {
    if (!tint_stack.empty()) {
        *color = tint_stack.front();
    } else {
        color->red = 1.0f;
        color->green = 1.0f;
        color->blue = 1.0f;
        color->alpha = 1.0f;
    }
}

void ltRestoreTint() {
    LTColor c;
    if (!tint_stack.empty()) {
        c = tint_stack.front();
    }
    ltColor(c.red, c.green, c.blue, c.alpha);
}

void ltPushBlendMode(LTBlendMode mode) {
    blend_mode_stack.push_front(mode);
    ltBlendMode(mode);
}

void ltPopBlendMode() {
    if (!blend_mode_stack.empty()) {
        blend_mode_stack.pop_front();
        if (!blend_mode_stack.empty()) {
            ltBlendMode(blend_mode_stack.front());
        } else {
            ltBlendMode(LT_BLEND_MODE_NORMAL);
        }
    }
}

void ltPushTextureMode(LTTextureMode mode) {
    texture_mode_stack.push_front(mode);
    ltTextureMode(mode);
}

void ltPopTextureMode() {
    if (!texture_mode_stack.empty()) {
        texture_mode_stack.pop_front();
        if (!texture_mode_stack.empty()) {
            ltTextureMode(texture_mode_stack.front());
        } else {
            ltTextureMode(LT_TEXTURE_MODE_MODULATE);
        }
    }
}

void ltDrawUnitSquare() {
    static const LTfloat vertices[] = {-0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f};

    ltDisableTextures();
    ltBindVertBuffer(0);
    ltVertexPointer(2, LT_VERT_DATA_TYPE_FLOAT, 0, (void*)vertices);
    ltDrawArrays(LT_DRAWMODE_TRIANGLE_FAN, 0, 4);
}

void ltDrawUnitCircle() {
    static const int num_vertices = 128;
    LTfloat vertices[num_vertices * 2];

    for (int i = 0; i < num_vertices * 2; i += 2) {
        LTfloat theta = ((float)i / (float)num_vertices) * 2.0f * LT_PI;
        vertices[i] = (LTfloat)cosf(theta);
        vertices[i + 1] = (LTfloat)sinf(theta);
    }

    ltDisableTextures();
    ltBindVertBuffer(0);
    ltVertexPointer(2, LT_VERT_DATA_TYPE_FLOAT, 0, vertices);
    ltDrawArrays(LT_DRAWMODE_TRIANGLE_FAN, 0, num_vertices);
}

void ltDrawRect(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2) {
    LTfloat vertices[8];
    vertices[0] = x1;
    vertices[1] = y1;
    vertices[2] = x2;
    vertices[3] = y1;
    vertices[4] = x2;
    vertices[5] = y2;
    vertices[6] = x1;
    vertices[7] = y2;
    ltDisableTextures();
    ltBindVertBuffer(0);
    ltVertexPointer(2, LT_VERT_DATA_TYPE_FLOAT, 0, vertices);
    ltDrawArrays(LT_DRAWMODE_TRIANGLE_FAN, 0, 4);
}

void ltDrawEllipse(LTfloat x, LTfloat y, LTfloat rx, LTfloat ry) {
    ltPushMatrix();
        ltTranslate(x, y, 0.0f);
        ltScale(rx, ry, 1.0f);
        ltDrawUnitCircle();
    ltPopMatrix();
}

void ltDrawPoly(LTfloat *vertices, int num_vertices) {
    ltDisableTextures();
    ltBindVertBuffer(0);
    ltVertexPointer(2, LT_VERT_DATA_TYPE_FLOAT, 0, vertices);
    ltDrawArrays(LT_DRAWMODE_TRIANGLE_FAN, 0, num_vertices);
}

void ltDrawLineStrip(LTfloat *vertices, int num_vertices) {
    ltDisableTextures();
    ltBindVertBuffer(0);
    ltVertexPointer(2, LT_VERT_DATA_TYPE_FLOAT, 0, vertices);
    ltDrawArrays(LT_DRAWMODE_LINE_STRIP, 0, num_vertices);
}
