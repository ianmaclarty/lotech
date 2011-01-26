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

#include <list>
#include <map>

#include <string.h>

#include "ltcommon.h"
#include "ltevent.h"

#define LT_PI 3.14159265358979323846f
#define LT_RADIANS_PER_DEGREE (LT_PI / 180.0f)
#define LT_DEGREES_PER_RADIAN (180.0f / LT_PI)

typedef LTuint32        LTpixel;
typedef GLuint          LTvertbuf;
typedef GLuint          LTtexbuf;

#define LT_ALPHA(pxl)     (pxl & 0xFF)


void ltInitGraphics();

struct LTAtlas;
void ltEnableAtlas(LTAtlas *atlas);
void ltDisableTextures();

void ltSetViewPort(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2);

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

//---------------------------------------------------------------------
// Props (drawable objects).

struct LTProp : LTObject {
    std::list<LTPointerEventHandler *> *event_handlers;

    LTProp(LTType type);
    virtual ~LTProp();

    virtual void draw() = 0;

    // Call all the event handles for this node only, returning true iff at least one
    // of the event handlers returns true.
    bool consumePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    // Propogate an event to this and all descendent nodes, calling event
    // handlers along the way.  If at least one of the event handlers for a particular
    // node returns true, then propogation stops.
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    // Returns true iff this node, or one of its descendents, contains the given point.
    virtual bool containsPoint(LTfloat x, LTfloat y) { return false; } // XXX make abstract.

    void addHandler(LTPointerEventHandler *handler);
};

struct LTTranslate2D : LTProp {
    LTfloat x;
    LTfloat y;
    LTProp *target;

    LTTranslate2D(LTfloat x, LTfloat y, LTProp *target);
    virtual ~LTTranslate2D();

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTRotate2D : LTProp {
    LTdegrees angle;
    LTProp *target;

    LTRotate2D(LTdegrees angle, LTProp *target);
    virtual ~LTRotate2D();

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTScale2D : LTProp {
    LTfloat sx;
    LTfloat sy;
    LTProp *target;

    LTScale2D(LTfloat sx, LTfloat sy, LTProp *target);
    virtual ~LTScale2D();

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTTint : LTProp {
    LTfloat r;
    LTfloat g;
    LTfloat b;
    LTfloat a;
    LTProp *target;

    LTTint(LTfloat r, LTfloat g, LTfloat b, LTfloat a, LTProp *target);
    virtual ~LTTint();

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);

    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTScene : LTProp {
    std::multimap<LTfloat, LTProp*> scene;
    std::multimap<LTProp*, std::multimap<LTfloat, LTProp*>::iterator> prop_index;

    LTScene();
    virtual ~LTScene();

    void insert(LTProp *prop, LTfloat depth);
    void remove(LTProp *prop);

    virtual void draw();
    virtual bool propogatePointerEvent(LTfloat x, LTfloat y, LTPointerEvent *event);
};

struct LTLine : LTProp {
    LTfloat x1, y1, x2, y2;

    LTLine(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2);
    
    virtual void draw();

    virtual LTfloat* field_ptr(const char *field_name);
};

struct LTTriangle : LTProp {
    LTfloat x1, y1, x2, y2, x3, y3;

    LTTriangle(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2, LTfloat x3, LTfloat y3);
    
    virtual void draw();

    virtual LTfloat* field_ptr(const char *field_name);
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

enum LTAnchor {
    LT_ANCHOR_CENTER,
    LT_ANCHOR_BOTTOM_LEFT
};

struct LTAtlas : LTObject {
    GLuint texture_id;

    LTAtlas(LTImagePacker *packer, const char *dump_file = NULL);
    virtual ~LTAtlas();
};

struct LTImage : LTProp {
    LTAtlas   *atlas;
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

    // Original image size, as ratio of atlas dimensions.
    LTfloat   orig_width;
    LTfloat   orig_height;

    bool rotated;

    // Dimensions of original image in pixels.
    int       pixel_width;
    int       pixel_height;

    // packer->occupant != NULL
    LTImage(LTAtlas *atlas, int atlas_w, int atlas_h, LTImagePacker *packer);
    virtual ~LTImage();

    virtual void draw();

    void setAnchor(LTAnchor anchor);
};

#endif
