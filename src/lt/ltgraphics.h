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
typedef GLuint          LTvertbuf;
typedef GLuint          LTtexbuf;

#define LT_RED(pxl)     (pxl & 0xFF)
#define LT_GREEN(pxl)   (pxl >> 8 & 0xFF)
#define LT_BLUE(pxl)    (pxl >> 16 & 0xFF)
#define LT_ALPHA(pxl)   (pxl >> 24)

void ltInitGraphics();

void ltEnableTexture(LTtexture tex);
void ltDisableTextures();

void ltSetViewPort(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2);

void ltSetColor(LTfloat r, LTfloat g, LTfloat b, LTfloat a);
void ltTranslate(LTfloat x, LTfloat y);
void ltRotate(LTdegrees degrees);
void ltScale(LTfloat x, LTfloat y, LTfloat z);
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
LTImageBuffer *ltReadImage(const char *file);

/* Only the bounding box is written. */
void ltWriteImage(const char *file, LTImageBuffer *img);

/*
 * x and y are left-bottom coordinates to paste src in dest, relative to 
 * dest->bb_left and dest->bb_bottom.
 */
void ltPasteImage(LTImageBuffer *src, LTImageBuffer *dest, int x, int y, bool rotate);

// Invariant: occupant != NULL <=> hi_child != NULL && lo_child != NULL.
struct LTImagePacker {
    int left;
    int bottom;
    int width;
    int height;
    LTImageBuffer *occupant;
    bool rotated; // 90 degrees clockwise, so that the lower-left corner becomes the top-left corner.
    LTImagePacker *hi_child;
    LTImagePacker *lo_child;

    LTImagePacker(int l, int b, int w, int h);
    virtual ~LTImagePacker();

    void deleteOccupants();
    int size();
};

/* Returns false if there's no room in the bin. */
bool ltPackImage(LTImagePacker *packer, LTImageBuffer *img);

/* The caller is responsible for freeing the buffer (with delete). */
LTImageBuffer *ltCreateAtlasImage(const char *file, LTImagePacker *packer);

/* The caller is responsible for freeing the texture with ltDeleteTexture. */
LTtexture ltCreateAtlasTexture(LTImagePacker *packer);

void ltDeleteTexture(LTtexture);

struct LTImage {
    LTtexture atlas;
    LTvertbuf vertbuf;
    LTtexbuf  texbuf;

    // Texture coords of image bounding box in atlas.
    LTfloat   tex_left;
    LTfloat   tex_bottom;

    // Bounding box coords, as ratio of atlas dimensions.
    LTfloat   bb_left;
    LTfloat   bb_bottom;
    LTfloat   bb_width;
    LTfloat   bb_height;

    // Original image size, as ration of atlas dimensions.
    LTfloat   orig_width;
    LTfloat   orig_height;

    bool rotated;

    // Dimensions of original image in pixels.
    int       pxl_w;
    int       pxl_h;

    // packer->occupant != NULL
    LTImage(LTtexture atlas, int atlas_w, int atlas_h, LTImagePacker *packer);
    virtual ~LTImage();

    void drawBottomLeft();
};

#endif
