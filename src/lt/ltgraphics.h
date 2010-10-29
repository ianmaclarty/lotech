/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTGRAPHICS_H
#define LTGRAPHICS_H

#ifdef LINUX
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#else
#include <OpenGL/GL.h>
#endif

#include "ltcommon.h"

#define LT_PI 3.14159265358979323846f
#define LT_RADIANS_PER_DEGREE (LT_PI / 180.0f)
#define LT_DEGREES_PER_RADIAN (180.0f / LT_PI)

typedef LTuint32        LTpixel;
typedef GLuint          LTtexture;

#define LT_RED(pxl)     (pxl & 0xFF)
#define LT_GREEN(pxl)   (pxl >> 8 & 0xFF)
#define LT_BLUE(pxl)    (pxl >> 16 & 0xFF)
#define LT_ALPHA(pxl)   (pxl >> 24)

void ltInitGraphics();

void ltEnableTextures();
void ltDisableTextures();

void ltSetViewPort(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2);

void ltSetColor(LTfloat r, LTfloat g, LTfloat b, LTfloat a);
void ltTranslate(LTfloat x, LTfloat y);
void ltRotate(LTdegrees degrees);
void ltPushMatrix();
void ltPopMatrix();

void ltDrawUnitSquare();
void ltDrawUnitCircle();
void ltDrawRect(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2);
void ltDrawEllipse(LTfloat x, LTfloat y, LTfloat rx, LTfloat ry);
void ltDrawPoly(LTfloat *vertices, int num_vertices); /* Must be convex */

struct LTImageBuffer {
    // Dimensions of original image (not bounding box).
    int width;
    int height;

    // Bounding box.  Bottom-left corner is 0,0.  Top-right corner is width,height.
    int bb_left;
    int bb_top;
    int bb_right;
    int bb_bottom;

    /* Bounding box pixels from bottom-left to top-right, scanning left to right. */
    LTpixel *bb_pixels;

    const char *file; // For error messages.

    virtual ~LTImageBuffer();

    int bb_width();
    int bb_height();
    int num_bb_pixels();
};

/* The caller is responsible for freeing the buffer (with delete). */
LTImageBuffer *ltLoadImage(const char *file);

/* Only the bounding box is written. */
void ltWriteImage(const char *file, LTImageBuffer *img);

/*
 * x and y are left-bottom coordinates to paste src in dest, relative to 
 * dest->bb_left and dest->bb_bottom.
 */
void ltPasteImage(LTImageBuffer *src, LTImageBuffer *dest, int x, int y, bool rotate);

// Invariant: occupant != NULL <=> hi_child != NULL && lo_child != NULL.
struct LTPackBin {
    int left;
    int bottom;
    int width;
    int height;
    LTImageBuffer *occupant;
    bool rotated;
    LTPackBin *hi_child;
    LTPackBin *lo_child;

    LTPackBin(int l, int b, int w, int h);
    virtual ~LTPackBin();

    void deleteOccupants();
};

/* Returns false if there's no room in the bin. */
bool ltPackImage(LTPackBin *bin, LTImageBuffer *img);

/* Free all images in the bin. */
void ltDeleteImagesInBin(LTPackBin *bin);

/* The caller is responsible for freeing the buffer (with delete). */
LTImageBuffer *ltCreateAtlasImage(const char *file, LTPackBin *bin);

/* The caller is responsible for freeing the texture with ltDeleteTexture. */
LTtexture ltCreateAtlasTexture(LTPackBin *bin);

void ltDeleteTexture(LTtexture);

#endif
