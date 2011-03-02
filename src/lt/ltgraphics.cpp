/* Copyright (C) 2010 Ian MacLarty */
#include "ltgraphics.h"
#include "ltimage.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LTfloat screen_width = 100.0f;
static LTfloat screen_height = 100.0f;

static LTfloat viewport_left = -1.0f;
static LTfloat viewport_bottom = -1.0f;
static LTfloat viewport_right = 1.0f;
static LTfloat viewport_top = 1.0f;
static LTfloat viewport_width = 2.0f;
static LTfloat viewport_height = 2.0f;

static LTfloat pixel_width = viewport_width / screen_width;
static LTfloat pixel_height = viewport_height / screen_height;

struct LTColor {
    LTfloat r;
    LTfloat g;
    LTfloat b;
    LTfloat a;

    LTColor() {
        r = 1.0f;
        g = 1.0f;
        b = 1.0f;
        a = 1.0f;
    }
};

#define LT_TINT_STACK_SIZE 64
static LTColor tint_stack[LT_TINT_STACK_SIZE];
static int tint_stack_top = 0;

void ltInitGraphics() {
    glDisable(GL_DITHER);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_FOG);
    ltDisableTextures();
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    tint_stack_top = 0;
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_INDEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, (int)screen_width, (int)screen_height);
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
    pixel_width = viewport_width / screen_width;
    pixel_height = viewport_height / screen_height;
}

void ltSetScreenSize(int width, int height) {
    screen_width = (LTfloat)width;
    screen_height = (LTfloat)height;
    pixel_width = viewport_width / screen_width;
    pixel_height = viewport_height / screen_height;
}

void ltResizeScreen(int width, int height) {
    screen_width = (LTfloat)width;
    screen_height = (LTfloat)height;
}

LTfloat ltGetPixelWidth() {
    return pixel_width;
}

LTfloat ltGetPixelHeight() {
    return pixel_height;
}

LTfloat ltGetViewPortX(LTfloat screen_x) {
    return viewport_left + (screen_x / screen_width) * viewport_width;
}

LTfloat ltGetViewPortY(LTfloat screen_y) {
    return viewport_top - (screen_y / screen_height) * (viewport_height);
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
    if (tint_stack_top < (LT_TINT_STACK_SIZE - 1)) {
        tint_stack_top++;
        LTColor *top = &tint_stack[tint_stack_top];
        *top = *(top - 1);
        top->r *= r;
        top->g *= g;
        top->b *= b;
        top->a *= a;
        glColor4f(top->r, top->g, top->b, top->a);
    }
}

void ltPopTint() {
    if (tint_stack_top > 0) {
        tint_stack_top--;
        LTColor *top = &tint_stack[tint_stack_top];
        glColor4f(top->r, top->g, top->b, top->a);
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
