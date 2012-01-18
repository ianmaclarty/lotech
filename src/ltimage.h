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

#ifdef LTGLES1
    #define LT_PIXEL_VISIBLE(pxl)     (pxl & 0xFF000000)
#else
    #define LT_PIXEL_VISIBLE(pxl)     (pxl & 0xFF)
#endif

enum LTTextureFilter {
    LT_TEXTURE_FILTER_LINEAR,
    LT_TEXTURE_FILTER_NEAREST,
};

struct LTAtlas;

void ltEnableAtlas(LTAtlas *atlas);
void ltEnableTexture(LTtexid texture_id);
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
    LTfloat scaling;

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
    int max_size;
    LTImageBuffer *occupant;
    bool rotated; // 90 degrees clockwise, so that the lower-left corner becomes the top-left corner.
    LTImagePacker *hi_child;
    LTImagePacker *lo_child;

    LTImagePacker(int l, int b, int w, int h, int max_size);
    virtual ~LTImagePacker();

    void deleteOccupants();
    void clear(); // Just removes occupants without freeing them.
    int size();

    // Populate given array with images in packer.  imgs should
    // have enough space to hold size() images.
    void getImages(LTImageBuffer **imgs);
};

/* Returns false if there's no room in the packer. */
bool ltPackImage(LTImagePacker *packer, LTImageBuffer *img);

/* The caller is responsible for freeing the buffer (with delete). */
LTImageBuffer *ltCreateAtlasImage(const char *name, LTImagePacker *packer);

/* The caller is responsible for freeing the buffer (with delete). */
LTImageBuffer *ltCreateEmptyImageBuffer(const char *name, int w, int h);

struct LTAtlas {
    LTtexid texture_id;
    int ref_count;

    LTAtlas(LTImagePacker *packer, LTTextureFilter minfilter, LTTextureFilter magfilter);
    virtual ~LTAtlas();
};

struct LTImage : LTSceneNode {
    LTAtlas   *atlas;
    LTvertbuf vertbuf;
    LTtexbuf  texbuf;

    // Texture coords of image bounding box in atlas.
    LTtexcoord   tex_coords[8];

    // Bounding box dimensions in world coordinates.
    LTfloat   world_vertices[8];
    LTfloat   bb_width;
    LTfloat   bb_height;

    // Original image size, in world coordinates.
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
    virtual bool has_field(const char *field_name);
    virtual LTfloat get_field(const char *field_name);
    virtual LTfloat* field_ptr(const char *field_name);
};

#endif
