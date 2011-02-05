/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTIMAGE_H
#define LTIMAGE_H

#include <list>
#include <map>

#include <string.h>

#include "ltcommon.h"
#include "ltevent.h"
#include "ltscene.h"
#include "ltgraphics.h"

#ifdef IOS
    #define LT_PIXEL_VISIBLE(pxl)     (pxl & 0xFF000000)
#else
    #define LT_PIXEL_VISIBLE(pxl)     (pxl & 0xFF)
#endif

struct LTAtlas;

void ltEnableAtlas(LTAtlas *atlas);
void ltDisableTextures();

struct LTImageBuffer {
    // Dimensions of original image (not bounding box).
    int width;
    int height;

    // Bounding box.
    int bb_left;
    int bb_top;
    int bb_right;
    int bb_bottom;

    /* Bounding box pixels from bottom-left to top-right, scanning left to right. */
    LTpixel *bb_pixels;

    char *name; // Name used in lua code (usually basename of png file without extension).
    bool is_glyph;
    char glyph_char;

    // LTImageBuffer will make a copy of the filename string.
    LTImageBuffer(const char *file);
    virtual ~LTImageBuffer();

    int bb_width();
    int bb_height();
    int num_bb_pixels();
};

/*
 * The caller is responsible for freeing the buffer (with delete).
 * path is the full path to the file.  name is what is set in
 * the generated LTImageBuffer file for use when generating a
 * Lua table.
 */
LTImageBuffer *ltReadImage(const char *path, const char *name);

/* Only the bounding box is written. */
void ltWriteImage(const char *path, LTImageBuffer *img);

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
LTImageBuffer *ltCreateAtlasImage(const char *name, LTImagePacker *packer);

enum LTAnchor {
    LT_ANCHOR_CENTER,
    LT_ANCHOR_BOTTOM_LEFT
};

struct LTAtlas {
    GLuint texture_id;
    int num_live_images;

    LTAtlas(LTImagePacker *packer, const char *dump_file = NULL);
    virtual ~LTAtlas();
};

struct LTImage : LTSceneNode {
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
    virtual LTfloat* field_ptr(const char *field_name);

    void setAnchor(LTAnchor anchor);
};

#endif
