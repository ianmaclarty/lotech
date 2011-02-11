/* Copyright (C) 2010 Ian MacLarty */
#include "ltgraphics.h"
#include "ltimage.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
static LTColor g_tint_stack[LT_TINT_STACK_SIZE];
static int g_tint_stack_top = 0;

void ltInitGraphics() {
    glDisable(GL_DITHER);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_FOG);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ltSetViewPort(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2) {
    glMatrixMode(GL_PROJECTION);
    #ifdef IOS
    glOrthof(x1, x2, y1, y2, -1.0f, 1.0f);
    #else
    glOrtho(x1, x2, y1, y2, -1.0f, 1.0f);
    #endif
    glMatrixMode(GL_MODELVIEW);
}

void ltPushTint(LTfloat r, LTfloat g, LTfloat b, LTfloat a) {
    if (g_tint_stack_top < (LT_TINT_STACK_SIZE - 1)) {
        g_tint_stack_top++;
        LTColor *top = &g_tint_stack[g_tint_stack_top];
        *top = *(top - 1);
        top->r *= r;
        top->g *= g;
        top->b *= b;
        top->a *= a;
        glColor4f(top->r, top->g, top->b, top->a);
    }
}

void ltPopTint() {
    if (g_tint_stack_top > 0) {
        g_tint_stack_top--;
        LTColor *top = &g_tint_stack[g_tint_stack_top];
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
