/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTGRAPHICS_H
#define LTGRAPHICS_H

#include <list>
#include <map>
#include <math.h>

#include <string.h>

#include "ltcommon.h"
#include "ltevent.h"

#define LT_MAX_TEX_COORD 8192

struct LTPoint {
    LTfloat x;
    LTfloat y;

    LTPoint(LTfloat x, LTfloat y) {
        LTPoint::x = x;
        LTPoint::y = y;
    }

    LTPoint() {
        x = 0.0f;
        y = 0.0f;
    }

    void normalize() {
        if (x != 0.0f || y != 0.0f) {
            LTfloat l = len();
            x /= l;
            y /= l;
        }
    }

    LTfloat len() {
        return sqrtf(x * x + y * y);
    }
};

struct LTCompactColor {
    GLubyte r;
    GLubyte g;
    GLubyte b;
    GLubyte a;

    LTCompactColor() {
        r = 255;
        g = 255;
        b = 255;
        a = 255;
    }

    LTCompactColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a) {
        LTCompactColor::r = r;
        LTCompactColor::g = g;
        LTCompactColor::b = b;
        LTCompactColor::a = a;
    }
};

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

enum LTDisplayOrientation {
    LT_DISPLAY_ORIENTATION_PORTRAIT,
    LT_DISPLAY_ORIENTATION_LANDSCAPE,
};

enum LTBlendMode {
    LT_BLEND_MODE_NORMAL,
    LT_BLEND_MODE_ADD,
    LT_BLEND_MODE_COLOR,
    LT_BLEND_MODE_OFF,
};

enum LTTextureMode {
    LT_TEXTURE_MODE_MODULATE,
    LT_TEXTURE_MODE_ADD,
    LT_TEXTURE_MODE_DECAL,
    LT_TEXTURE_MODE_BLEND,
    LT_TEXTURE_MODE_REPLACE,
};

// Should be called before rendering each frame.
void ltInitGraphics();
void ltPrepareForRendering(
    int screen_viewport_x, int screen_viewport_y,
    int screen_viewport_width, int screen_viewport_height,
    LTfloat viewport_left, LTfloat viewport_bottom,
    LTfloat viewport_right, LTfloat viewport_top,
    LTColor *clear_color, bool clear_depthbuf);
void ltFinishRendering();

// The following functions should be called only once.
void ltSetViewPort(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2);
void ltSetScreenSize(int width, int height);
void ltSetDesignScreenSize(LTfloat width, LTfloat height);
void ltSetDisplayOrientation(LTDisplayOrientation orientation);
void ltAdjustViewportAspectRatio();

// This should be called whenever the screen is resized.
void ltResizeScreen(int width, int height);

// Dimensions of a pixel in viewport coords.
LTfloat ltGetPixelWidth();
LTfloat ltGetPixelHeight();

// Convert screen coords to world coords.
LTfloat ltGetViewPortX(LTfloat screen_x);
LTfloat ltGetViewPortY(LTfloat screen_y);

LTfloat ltGetViewPortLeftEdge();
LTfloat ltGetViewPortRightEdge();
LTfloat ltGetViewPortBottomEdge();
LTfloat ltGetViewPortTopEdge();

LTDisplayOrientation ltGetDisplayOrientation();

void ltPushPerspective(LTfloat near, LTfloat origin, LTfloat far);
void ltPopPerspective();

void ltPushTint(LTfloat r, LTfloat g, LTfloat b, LTfloat a);
void ltPopTint();
void ltPeekTint(LTColor *color);
void ltRestoreTint();
void ltPushBlendMode(LTBlendMode mode);
void ltPopBlendMode();
void ltPushTextureMode(LTTextureMode mode);
void ltPopTextureMode();
void ltTranslate(LTfloat x, LTfloat y, LTfloat z);
void ltRotate(LTdegrees degrees, LTfloat x, LTfloat y, LTfloat z);
void ltScale(LTfloat x, LTfloat y, LTfloat z);
void ltPushMatrix();
void ltPopMatrix();
void ltMultMatrix(LTfloat *m);

void ltDrawUnitSquare();
void ltDrawUnitCircle();
void ltDrawRect(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2);
void ltDrawEllipse(LTfloat x, LTfloat y, LTfloat rx, LTfloat ry);
void ltDrawPoly(LTfloat *vertices, int num_vertices); /* Must be convex */

void ltDrawAdBackground();
#endif
