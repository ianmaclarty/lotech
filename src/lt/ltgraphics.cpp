/* Copyright (C) 2010 Ian MacLarty */
#include "ltgraphics.h"
#include "ltimage.h"
#ifdef LTIOS
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

// Dimensions of a (design) pixel in user space.
// Used to decide how to scale images by default.
static LTfloat pixel_width = viewport_width / design_width;
static LTfloat pixel_height = viewport_height / design_height;

static LTDisplayOrientation display_orientation = LT_DISPLAY_ORIENTATION_LANDSCAPE;

struct LTColor {
    LTfloat r;
    LTfloat g;
    LTfloat b;
    LTfloat a;

    LTColor(LTfloat r, LTfloat g, LTfloat b, LTfloat a) {
        LTColor::r = r;
        LTColor::g = g;
        LTColor::b = b;
        LTColor::a = a;
    }

    LTColor() {
        r = 1.0f;
        g = 1.0f;
        b = 1.0f;
        a = 1.0f;
    }
};

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
    glViewport(0, 0, screen_width, screen_height);
    #ifdef LTIOS
    glOrthof(viewport_left, viewport_right, viewport_bottom, viewport_top, -1.0f, 1.0f);
    #else
    glOrtho(viewport_left, viewport_right, viewport_bottom, viewport_top, -1.0f, 1.0f);
    #endif
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void ltSetViewPort(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2) {
    viewport_left = x1;
    viewport_right = x2;
    viewport_bottom = y1;
    viewport_top = y2;
    viewport_width = viewport_right - viewport_left;
    viewport_height = viewport_top - viewport_bottom;
    pixel_width = viewport_width / design_width;
    pixel_height = viewport_height / design_height;
}

void ltAdjustViewportAspectRatio() {
    LTfloat actual_w_over_h;
    if (display_orientation == LT_DISPLAY_ORIENTATION_LANDSCAPE
        && screen_width < screen_height
        || display_orientation == LT_DISPLAY_ORIENTATION_PORTRAIT
        && screen_width > screen_height)
    {
        actual_w_over_h = (LTfloat)screen_height / (LTfloat)screen_width;
    } else {
        actual_w_over_h = (LTfloat)screen_width / (LTfloat)screen_height;
    }
    LTfloat design_w_over_h = design_width / design_height;
    LTfloat distortion = fabsf(design_w_over_h - actual_w_over_h);
    if (distortion > 0.01f) {
        if (actual_w_over_h < design_w_over_h) {
            // too fat
            viewport_bottom -= viewport_height * distortion * 0.5f;
            viewport_top += viewport_height * distortion * 0.5f;
            viewport_height = viewport_top - viewport_bottom;
        } else {
            // too thin
            viewport_left -= viewport_width * distortion * 0.5f;
            viewport_right += viewport_width * distortion * 0.5f;
            viewport_width = viewport_right - viewport_left;
        }
    }
}

void ltSetScreenSize(int width, int height) {
    screen_width = width;
    screen_height = height;
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
    return viewport_left + ((screen_x * scaling) / (LTfloat)screen_width) * viewport_width;
#else
    return viewport_left + (screen_x / (LTfloat)screen_width) * viewport_width;
#endif
}

LTfloat ltGetViewPortY(LTfloat screen_y) {
#ifdef LTIOS
    static LTfloat scaling = 0.0f;
    if (scaling == 0.0f) {
        scaling = ltIOSScaling();
    }
    return viewport_top - ((screen_y * scaling) / (LTfloat)screen_height) * viewport_height;
#else
    return viewport_top - (screen_y / (LTfloat)screen_height) * viewport_height;
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
    LTfloat dx = r * viewport_width * 0.5;
    LTfloat dy = r * viewport_height * 0.5;
    #ifdef LTIOS
        glFrustumf(viewport_left + dx, viewport_right - dx, viewport_bottom + dy, viewport_top - dy, near, far);
    #else
        glFrustum(viewport_left + dx, viewport_right - dx, viewport_bottom + dy, viewport_top - dy, near, far);
    #endif
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -origin);
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
