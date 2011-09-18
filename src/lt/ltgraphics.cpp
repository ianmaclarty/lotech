/* Copyright (C) 2010 Ian MacLarty */
#include "ltgraphics.h"
#include "ltimage.h"
#ifdef LTIOS
#   include "ltads.h"
#   include "ltiosutil.h"
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Actual screen dimensions in pixels.
static int screen_width = 480;
static int screen_height = 320;

// Position of the glViewport (in pixels).
static int screen_viewport_x = 0;
static int screen_viewport_y = 0;
static int screen_viewport_width = 480;
static int screen_viewport_height = 320;

// Screen dimensions used to compute size of pixels in loaded
// images.  These don't have to match the actual screen dimensions.
static LTfloat design_width = 960.0f;
static LTfloat design_height = 640.0f;

// User space dimensions.
static LTfloat viewport_left = -1.0f;
static LTfloat viewport_bottom = -1.0f;
static LTfloat viewport_right = 1.0f;
static LTfloat viewport_top = 1.0f;
static LTfloat viewport_width = 2.0f;
static LTfloat viewport_height = 2.0f;

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

void ltInitGraphics() {
    glDisable(GL_DITHER);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_FOG);
    ltDisableTextures();
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    GLbitfield clear_mask = GL_COLOR_BUFFER_BIT;
    #ifdef LTDEPTHBUF
        clear_mask |= GL_DEPTH_BUFFER_BIT;
    #endif
    glClear(clear_mask);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    tint_stack.clear();
    blend_mode_stack.clear();
    glEnableClientState(GL_VERTEX_ARRAY);
    #ifndef LTIOS
    glEnableClientState(GL_INDEX_ARRAY);
    #endif
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(screen_viewport_x, screen_viewport_y, screen_viewport_width, screen_viewport_height);
    #ifdef LTIOS
    glOrthof(viewport_left, viewport_right, viewport_bottom, viewport_top, -1.0f, 1.0f);
    #else
    glOrtho(viewport_left, viewport_right, viewport_bottom, viewport_top, -1.0f, 1.0f);
    #endif
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
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

void ltAdjustViewportAspectRatio() {
    LTfloat w0 = design_width;
    LTfloat h0 = design_height;
    LTfloat w1 = (LTfloat)screen_width;
    LTfloat h1 = (LTfloat)screen_height;
    #ifdef LTENVELOPE
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
    #else
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
            if (dy > 0.01) {
                viewport_bottom = design_viewport_bottom - design_viewport_height * dy;
                viewport_top = design_viewport_top + design_viewport_height * dy;
            } else {
                viewport_bottom = design_viewport_bottom;
                viewport_top = design_viewport_top;
            }
        }
        viewport_width = viewport_right - viewport_left;
        viewport_height = viewport_top - viewport_bottom;
    #endif

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
    /*
    LTfloat ct = 0.8f;
    LTfloat cb = 0.1f;
    //LTfloat u = b - (t - b) / 20.0f;
    LTfloat v[] = {
        l, t, ct, ct, ct, 1.0f,
        r, t, ct, ct, ct, 1.0f,
        l, b, cb, cb, cb, 1.0f,
        r, b, cb, cb, cb, 1.0f,
        //l, u, cb, cb, cb, 0.0f,
        //r, u, cb, cb, cb, 0.0f,
    };
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    LTfloat stride = 6 * sizeof(LTfloat);
    glVertexPointer(2, GL_FLOAT, stride, v);
    ltDisableTextures();
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, stride, v + 2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableClientState(GL_COLOR_ARRAY);
    */
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

void ltPushPerspective(LTfloat near, LTfloat origin, LTfloat far) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    LTfloat r = (origin - near) / origin; 
    LTfloat near_half_width = 0.5f * (viewport_width - r * viewport_width);
    LTfloat near_half_height = 0.5f * (viewport_height - r * viewport_height);
    #ifdef LTIOS
        glFrustumf(-near_half_width, near_half_width, -near_half_height, near_half_height, near, far);
    #else
        glFrustum(-near_half_width, near_half_width, -near_half_height, near_half_height, near, far);
    #endif
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(-(viewport_width * 0.5f + viewport_left), -(viewport_height * 0.5f + viewport_bottom), -origin);
}

void ltPopPerspective() {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void ltPushTint(LTfloat r, LTfloat g, LTfloat b, LTfloat a) {
    LTColor new_top(r, g, b, a);
    if (!tint_stack.empty()) {
        LTColor *top = &tint_stack.front();
        new_top.r *= top->r;
        new_top.g *= top->g;
        new_top.b *= top->b;
        new_top.a *= top->a;
    }
    tint_stack.push_front(new_top);
    glColor4f(new_top.r, new_top.g, new_top.b, new_top.a);
}

void ltPopTint() {
    if (!tint_stack.empty()) {
        tint_stack.pop_front();
        if (!tint_stack.empty()) {
            LTColor *top = &tint_stack.front();
            glColor4f(top->r, top->g, top->b, top->a);
        } else {
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
}

static void apply_blend_mode(LTBlendMode mode) {
    switch (mode) {
        case LT_BLEND_MODE_NORMAL:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case LT_BLEND_MODE_ADD:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
    }
}

void ltPushBlendMode(LTBlendMode mode) {
    blend_mode_stack.push_front(mode);
    apply_blend_mode(mode);
}

void ltPopBlendMode() {
    if (!blend_mode_stack.empty()) {
        blend_mode_stack.pop_front();
        if (!blend_mode_stack.empty()) {
            apply_blend_mode(blend_mode_stack.front());
        } else {
            apply_blend_mode(LT_BLEND_MODE_NORMAL);
        }
    }
}

void ltDrawUnitSquare() {
    static bool initialized = false;
    static const GLfloat vertices[] = {-0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f};
    static GLuint buffer_id;

    if (!initialized) {
        glGenBuffers(1, &buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, vertices, GL_STATIC_DRAW);
        initialized = true;
    }

    ltDisableTextures();
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    glVertexPointer(2, GL_FLOAT, 0, 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void ltDrawUnitCircle() {
    static bool initialized = false;
    static const int num_vertices = 128;
    static GLfloat vertices[num_vertices * 2];
    static GLuint buffer_id;

    if (!initialized) {
        for (int i = 0; i < num_vertices * 2; i += 2) {
            float theta = ((float)i / (float)num_vertices) * 2.0f * LT_PI;
            vertices[i] = (GLfloat)cosf(theta);
            vertices[i + 1] = (GLfloat)sinf(theta);
        }
        glGenBuffers(1, &buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * num_vertices * 2, vertices, GL_STATIC_DRAW);
        initialized = true;
    }

    ltDisableTextures();
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    glVertexPointer(2, GL_FLOAT, 0, 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, num_vertices);
}

void ltDrawRect(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2) {
    GLfloat vertices[8];
    vertices[0] = x1;
    vertices[1] = y1;
    vertices[2] = x2;
    vertices[3] = y1;
    vertices[4] = x2;
    vertices[5] = y2;
    vertices[6] = x1;
    vertices[7] = y2;
    ltDisableTextures();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void ltDrawEllipse(LTfloat x, LTfloat y, LTfloat rx, LTfloat ry) {
    glPushMatrix();
        glTranslatef(x, y, 0.0f);
        glScalef(rx, ry, 1.0f);
        ltDrawUnitCircle();
    glPopMatrix();
}

void ltDrawPoly(LTfloat *vertices, int num_vertices) {
    ltDisableTextures();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, num_vertices);
}

void ltTranslate(LTfloat x, LTfloat y, LTfloat z) {
    glTranslatef(x, y, z);
}

void ltRotate(LTdegrees degrees, LTfloat x, LTfloat y, LTfloat z) {
    glRotatef(degrees, x, y, z);
}

void ltScale(LTfloat x, LTfloat y, LTfloat z) {
    glScalef(x, y, z);
}

void ltPushMatrix() {
    glPushMatrix();
}

void ltPopMatrix() {
    glPopMatrix();
}

void ltMultMatrix(LTfloat *m) {
    glMultMatrixf(m);
}
