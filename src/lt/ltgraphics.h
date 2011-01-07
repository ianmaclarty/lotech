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

#include <string.h>

#include "ltcommon.h"

#define LT_PI 3.14159265358979323846f
#define LT_RADIANS_PER_DEGREE (LT_PI / 180.0f)
#define LT_DEGREES_PER_RADIAN (180.0f / LT_PI)

typedef LTuint32        LTpixel;
typedef GLuint          LTtexture;
typedef GLuint          LTvertbuf;
typedef GLuint          LTtexbuf;

#define LT_ALPHA(pxl)     (pxl & 0xFF)

void ltInitGraphics();

void ltEnableTexture(LTtexture tex);
void ltDisableTextures();

void ltSetViewPort(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2);

void ltSetColor(LTfloat r, LTfloat g, LTfloat b, LTfloat a);
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

//---------------------------------------------------------------------
// Props (drawable objects).

struct LTProp : LTObject {
    virtual void draw() = 0;
};

struct LTTranslator : LTProp {
    LTfloat x;
    LTfloat y;
    LTfloat z;
    LTProp *target;

    LTTranslator(LTfloat x, LTfloat y, LTfloat z, LTProp *target) {
        LTTranslator::x = x;
        LTTranslator::y = y;
        LTTranslator::z = z;
        LTTranslator::target = target;
        target->retain();
    }

    virtual ~LTTranslator() {
        target->release();
    }

    virtual void draw() {
        ltPushMatrix();
        ltTranslate(x, y, z);
        target->draw();
        ltPopMatrix();
    }

    virtual void* field_ptr(const char *field_name) {
        if (strcmp(field_name, "x") == 0) {
            return &x;
        }
        if (strcmp(field_name, "y") == 0) {
            return &y;
        }
        if (strcmp(field_name, "z") == 0) {
            return &z;
        }
        return target->field_ptr(field_name);
    }
};

struct LTRotator : LTProp {
    LTdegrees angle;
    LTfloat rx;
    LTfloat ry;
    LTfloat rz;
    LTProp *target;

    LTRotator(LTdegrees angle, LTfloat rx, LTfloat ry, LTfloat rz, LTProp *target) {
        LTRotator::angle = angle;
        LTRotator::rx = rx;
        LTRotator::ry = ry;
        LTRotator::rz = rz;
        LTRotator::target = target;
        target->retain();
    }

    virtual ~LTRotator() {
        target->release();
    }

    virtual void draw() {
        ltPushMatrix();
        ltRotate(angle, rx, ry, rz);
        target->draw();
        ltPopMatrix();
    }

    virtual void* field_ptr(const char *field_name) {
        if (strcmp(field_name, "angle") == 0) {
            return &angle;
        }
        if (strcmp(field_name, "rx") == 0) {
            return &rx;
        }
        if (strcmp(field_name, "ry") == 0) {
            return &ry;
        }
        if (strcmp(field_name, "rz") == 0) {
            return &rz;
        }
        return target->field_ptr(field_name);
    }
};

struct LTScalor : LTProp {
    LTfloat sx;
    LTfloat sy;
    LTfloat sz;
    LTProp *target;

    LTScalor(LTfloat sx, LTfloat sy, LTfloat sz, LTProp *target) {
        LTScalor::sx = sx;
        LTScalor::sy = sy;
        LTScalor::sz = sz;
        LTScalor::target = target;
        target->retain();
    }

    virtual ~LTScalor() {
        target->release();
    }

    virtual void draw() {
        ltPushMatrix();
        ltScale(sx, sy, sz);
        target->draw();
        ltPopMatrix();
    }

    virtual void* field_ptr(const char *field_name) {
        if (strcmp(field_name, "sx") == 0) {
            return &sx;
        }
        if (strcmp(field_name, "sy") == 0) {
            return &sy;
        }
        if (strcmp(field_name, "sz") == 0) {
            return &sz;
        }
        return target->field_ptr(field_name);
    }
};

//---------------------------------------------------------------------
// Images.

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
    void clear(); // Just removes occupants without freeing them.
    int size();

    // Populate given array with images in packer.  imgs should
    // have enough space to hold size() images.
    void getImages(LTImageBuffer **imgs);
};

/* Returns false if there's no room in the bin. */
bool ltPackImage(LTImagePacker *packer, LTImageBuffer *img);

/* The caller is responsible for freeing the buffer (with delete). */
LTImageBuffer *ltCreateAtlasImage(const char *file, LTImagePacker *packer);

/* The caller is responsible for freeing the texture with ltDeleteTexture. */
/* Atlas will be dumped to dump_file if it isn't NULL. */
LTtexture ltCreateAtlasTexture(LTImagePacker *packer, const char *dump_file);

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
    int       pixel_width;
    int       pixel_height;

    // packer->occupant != NULL
    LTImage(LTtexture atlas, int atlas_w, int atlas_h, LTImagePacker *packer);
    virtual ~LTImage();

    void drawBottomLeft();
};

#endif
