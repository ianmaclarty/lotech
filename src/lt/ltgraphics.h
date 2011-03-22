/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTGRAPHICS_H
#define LTGRAPHICS_H

#include <list>
#include <map>

#include <string.h>

#include "ltcommon.h"
#include "ltevent.h"

enum LTDisplayOrientation {
    LT_DISPLAY_ORIENTATION_PORTRAIT,
    LT_DISPLAY_ORIENTATION_LANDSCAPE,
};

// Should be called before rendering each frame.
void ltInitGraphics();

// The following functions should be called only once.
void ltSetViewPort(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2);
void ltSetScreenSize(int width, int height);
void ltSetDisplayOrientation(LTDisplayOrientation orientation);

// This should be called whenever the screen is resized.
void ltResizeScreen(int width, int height);

// Dimensions of a pixel in viewport coords.
LTfloat ltGetPixelWidth();
LTfloat ltGetPixelHeight();

// Convert screen coords to world coords.
LTfloat ltGetViewPortX(LTfloat screen_x);
LTfloat ltGetViewPortY(LTfloat screen_y);

LTDisplayOrientation ltGetDisplayOrientation();

void ltPushPerspective(LTfloat near, LTfloat origin, LTfloat far);
void ltPopPerspective();

void ltPushTint(LTfloat r, LTfloat g, LTfloat b, LTfloat a);
void ltPopTint();
void ltTranslate(LTfloat x, LTfloat y, LTfloat z);
void ltRotate(LTdegrees degrees, LTfloat x, LTfloat y, LTfloat z);
void ltScale(LTfloat x, LTfloat y, LTfloat z);
void ltPushMatrix();
void ltPopMatrix();

void ltDrawUnitSquare();
void ltDrawUnitCircle();
void ltDrawRect(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2);
void ltDrawEllipse(LTfloat x, LTfloat y, LTfloat rx, LTfloat ry);
void ltDrawPoly(LTfloat *vertices, int num_vertices); /* Must be convex */

#endif
