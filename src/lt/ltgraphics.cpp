/* Copyright (C) 2010 Ian MacLarty */
#include "ltgraphics.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "png.h"

#define LTPI (3.14159265358979323846f)

static bool g_textures_enabled = false;

void ltInitGraphics() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ltEnableTextures() {
    if (!g_textures_enabled) {
        glEnable(GL_TEXTURE_2D);
        g_textures_enabled = true;
    }
}

void ltDisableTextures() {
    if (g_textures_enabled) {
        glDisable(GL_TEXTURE_2D);
        g_textures_enabled = false;
    }
}

void ltSetViewPort(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2) {
    glMatrixMode(GL_PROJECTION);
    glOrtho(x1, x2, y1, y2, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
}

void ltSetColor(LTfloat r, LTfloat g, LTfloat b, LTfloat a) {
    glColor4f(r, g, b, a);
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
            float theta = ((float)i / (float)num_vertices) * 2.0f * LTPI;
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

void ltTranslate(LTfloat x, LTfloat y) {
    glTranslatef(x, y, 0.0f);
}

void ltRotate(LTfloat radians) {
    glRotatef(radians * LT_DEGREES_PER_RADIAN, 0.0f, 0.0f, 1.0f);
}

void ltPushMatrix() {
    glPushMatrix();
}

void ltPopMatrix() {
    glPopMatrix();
}

//-----------------------------------------------------------------

LTImageBuffer::~LTImageBuffer() {
    delete[] bb_pixels;
}

int LTImageBuffer::bb_width() {
    return bb_right - bb_left + 1;
}

int LTImageBuffer::bb_height() {
    return bb_top - bb_bottom + 1;
}

int LTImageBuffer::num_bb_pixels() {
    return bb_width() * bb_height();
}

static void compute_bbox(const char *file, LTpixel **rows, int w, int h,
    int *bb_left, int *bb_top, int *bb_right, int *bb_bottom)
{
    int row;
    int col;
    LTpixel pxl;
    bool row_clear = true;
    bool found_bb_top = false;
    *bb_top = h - 1;
    *bb_left = w - 1;
    *bb_right = 0;
    *bb_bottom = 0;
    for (row = 0; row < h; row++) {
        row_clear = true;
        for (col = 0; col < w; col++) {
            pxl = rows[row][col];
            if ((LT_ALPHA(pxl)) > 0) { // not transparent
                row_clear = false;
                if (col < *bb_left) {
                    *bb_left = col;
                }
                if (col > *bb_right) {
                    *bb_right = col;
                }
            }
        }
        if (!row_clear) {
            if (!found_bb_top) {
                *bb_top = row;
                found_bb_top = true;
            }
            *bb_bottom = row;
        }
    }

    if (*bb_left > *bb_right || *bb_top > *bb_bottom) {
        fprintf(stderr, "Error: %s has no non-transparent pixels.\n", file);
        exit(1);
    }
}

LTImageBuffer *ltLoadImage(const char *file) {
    FILE *in;
    png_structp png_ptr; 
    png_infop info_ptr; 
    png_infop end_ptr; 
    unsigned char sig[8];
    bool has_alpha;

    png_uint_32 uwidth;
    png_uint_32 uheight;
    int width, height;
    int bit_depth;
    int color_type;

    int bb_left, bb_top, bb_right, bb_bottom;

    int png_transforms;

    png_byte **rows;

    in = fopen(file, "rb");
    if (!in) {
        fprintf(stderr, "Error: Unable to open %s for reading.\n", file);
        exit(1);
    }

    // Check for 8 byte signature.
    int n = fread(sig, 1, 8, in);
    if (n != 8) {
        fclose(in);
        ltAbort("Unable to read first 8 bytes of %s.", file);
    }
    if (!png_check_sig(sig, 8)) {
        fclose(in);
        ltAbort("%s has an invalid signature.", file);
    }
    
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    end_ptr = png_create_info_struct(png_ptr);

    png_init_io(png_ptr, in);
    png_set_sig_bytes(png_ptr, 8);

    // Read the data.
    png_transforms = PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING |
        PNG_TRANSFORM_GRAY_TO_RGB;
    png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
    png_read_png(png_ptr, info_ptr, png_transforms, NULL);
    fclose(in);
    png_get_IHDR(png_ptr, info_ptr, &uwidth, &uheight, &bit_depth, &color_type,
        NULL, NULL, NULL);
    width = (int)uwidth;
    height = (int)uheight;
    if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
        has_alpha = true;
    } else if (color_type == PNG_COLOR_TYPE_RGB) {
        has_alpha = false;
    } else {
        fprintf(stderr, "Error: %s is not RGBA or RGB.\n", file);
        exit(1);
    }
    if (bit_depth != 8) {
        fprintf(stderr, "Error: %s does not have bit depth 8.\n", file);
        exit(1);
    }
    rows = png_get_rows(png_ptr, info_ptr);

    // Compute the bounding box.
    if (has_alpha) {
        compute_bbox(file, (LTpixel**)rows, width, height, &bb_left, &bb_top, &bb_right,
            &bb_bottom);
    } else {
        // No alpha, so bbox calculation trivial.
        bb_left = 0;
        bb_top = 0;
        bb_right = width - 1;
        bb_bottom = height - 1;
    }
    
    // Copy data to new LTImageBuffer.

    LTImageBuffer *imgbuf = new LTImageBuffer();
    imgbuf->width = width;
    imgbuf->height = height;
    imgbuf->bb_left = bb_left;
    imgbuf->bb_top = height - bb_top - 1; // Normalize coordinate system.
    imgbuf->bb_right = bb_right;
    imgbuf->bb_bottom = height - bb_bottom - 1;
    imgbuf->file = file;
    
    int num_bb_pixels = imgbuf->num_bb_pixels();
    int bb_width = imgbuf->bb_width();
    LTpixel *pixels = new LTpixel[num_bb_pixels];

    LTpixel *pxl_ptr = pixels;
    for (int row = bb_bottom; row >= bb_top; row--) {
        memcpy(pxl_ptr, &rows[row][bb_left * 4], bb_width * 4);
        pxl_ptr += bb_width;
    }

    imgbuf->bb_pixels = pixels;

    // Free libpng data (including rows).
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);

    return imgbuf;
}

void ltWriteImage(const char *file, LTImageBuffer *img) {
    FILE *out;
    png_structp png_ptr; 
    png_infop info_ptr; 
    png_byte **rows;
    int bb_height = img->bb_height();
    int bb_width = img->bb_width();

    // Open the file.
    out = fopen(file, "wb");
    if (!out) {
        fprintf(stderr, "Error: Unable to open %s for writing.\n", file);
        exit(1);
    }

    // Setup.
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    png_init_io(png_ptr, out);
    png_set_IHDR(png_ptr, info_ptr, bb_width, bb_height,
        8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    // Tell libpng where the data is.
    rows = new png_byte*[bb_height];
    LTpixel *pxl_ptr = img->bb_pixels;
    for (int i = bb_height - 1; i >= 0; i--) {
        rows[i] = (png_byte*)pxl_ptr;
        pxl_ptr += bb_width;
    }
    png_set_rows(png_ptr, info_ptr, rows);

    // Write image.
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    // Free libpng data.
    png_destroy_write_struct(&png_ptr, &info_ptr);
    delete[] rows;
}

void ltPasteImage(LTImageBuffer *src, LTImageBuffer *dest, int x, int y, bool rotate) {
    int src_width;
    int src_height;
    src_width = src->bb_width();
    src_height = src->bb_height();
    int dest_width = dest->bb_width();
    int dest_height = dest->bb_height();
    if (!rotate && (x + src_width > dest_width)) {
        fprintf(stderr, "Error: %s too wide to be pasted into %s at x = %d.\n",
            src->file, dest->file, x);
        exit(1);
    }
    if (!rotate && (y + src_height > dest_height)) {
        fprintf(stderr, "Error: %s too high to be pasted into %s at y = %d.\n",
            src->file, dest->file, y);
        exit(1);
    }
    if (rotate && (x + src_height > dest_width)) {
        fprintf(stderr, "Error: %s too high to be pasted into %s at x = %d after rotation.\n",
            src->file, dest->file, x);
        exit(1);
    }
    if (rotate && (y + src_width > dest_height)) {
        fprintf(stderr, "Error: %s too wide to be pasted into %s at y = %d after rotation.\n",
            src->file, dest->file, y);
        exit(1);
    }

    LTpixel *dest_ptr = dest->bb_pixels + y * dest_width + x;

    if (rotate) {
        LTpixel *src_ptr = src->bb_pixels + src_width - 1;
        int src_row = 0;
        int src_col = src_width - 1;
        while (src_col >= 0) {
            *dest_ptr = *src_ptr;
            src_ptr += src_width;
            dest_ptr++;
            src_row++;
            if (src_row >= src_height) {
                src_col--;
                src_row = 0;
                dest_ptr += dest_width - src_height;
                src_ptr = src->bb_pixels + src_col;
            }
        }
    } else {
        LTpixel *src_ptr = src->bb_pixels;
        int src_row = 0;
        while (src_row < src_height) {
            memcpy(dest_ptr, src_ptr, src_width * 4);
            src_ptr += src_width;
            dest_ptr += dest_width;
            src_row++;
        }
    }
}

LTPackBin::LTPackBin(int l, int b, int w, int h) {
    left = l;
    bottom = b;
    width = w;
    height = h;
    occupant = NULL;
    rotated = false;
    hi_child = NULL;
    lo_child = NULL;
}

LTPackBin::~LTPackBin() {
    if (occupant != NULL) {
        delete hi_child;
        delete lo_child;
    }
}

bool ltPackImage(LTPackBin *bin, LTImageBuffer *img) {
    int bin_w = bin->width;
    int bin_h = bin->height;
    int img_w = img->bb_width();
    int img_h = img->bb_height();
    if (bin->occupant == NULL) {
        if (img_w <= bin_w && img_h <= bin_h) {
            bin->occupant = img;
            bin->rotated = false;
            bin->hi_child = new LTPackBin(bin->left, bin->bottom + img_h,
                bin_w, bin_h - img_h);
            bin->lo_child = new LTPackBin(bin->left + img_w, bin->bottom,
                bin_w - img_w, img_h);
            return true;
        }
        if (img_h <= bin_w && img_w <= bin_h) {
            bin->occupant = img;
            bin->rotated = true;
            bin->hi_child = new LTPackBin(bin->left, bin->bottom + img_w,
                bin_w, bin_h - img_w);
            bin->lo_child = new LTPackBin(bin->left + img_h, bin->bottom,
                bin_w - img_h, img_w);
            return true;
        }
        return false;
    }
    return ltPackImage(bin->lo_child, img) || ltPackImage(bin->hi_child, img);
}

void LTPackBin::deleteOccupants() {
    if (occupant != NULL) {
        delete occupant;
        hi_child->deleteOccupants();
        lo_child->deleteOccupants();
    }
}

static void paste_bin_images(LTImageBuffer *img, LTPackBin *bin) {
    if (bin->occupant != NULL) {
        ltPasteImage(bin->occupant, img, bin->left, bin->bottom, bin->rotated);
        paste_bin_images(img, bin->lo_child);
        paste_bin_images(img, bin->hi_child);
    }
}

LTImageBuffer *ltCreateAtlasImage(const char *file, LTPackBin *bin) {
    int num_pixels = bin->width * bin->height;
    LTImageBuffer *atlas = new LTImageBuffer();
    atlas->width=bin->width;
    atlas->height=bin->height;
    atlas->bb_left=0;
    atlas->bb_right=bin->width - 1;
    atlas->bb_top=bin->height - 1;
    atlas->bb_bottom=0;
    atlas->bb_pixels = new LTpixel[num_pixels];
    atlas->file = file;
    paste_bin_images(atlas, bin);
    return atlas;
}
