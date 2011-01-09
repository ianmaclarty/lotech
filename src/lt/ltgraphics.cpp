/* Copyright (C) 2010 Ian MacLarty */
#include "ltgraphics.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "png.h"

#define LTPI (3.14159265358979323846f)

static bool g_textures_enabled = false;
static LTtexture g_current_bound_texture = 0;

static LTfloat g_red = 1.0f;
static LTfloat g_green = 1.0f;
static LTfloat g_blue = 1.0f;
static LTfloat g_alpha = 1.0f;

void ltInitGraphics() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ltEnableTexture(LTtexture tex) {
    if (!g_textures_enabled) {
        glEnable(GL_TEXTURE_2D);
        g_textures_enabled = true;
    }
    if (g_current_bound_texture != tex) {
        glBindTexture(GL_TEXTURE_2D, tex);
        g_current_bound_texture = tex;
    }
}

void ltDisableTextures() {
    if (g_textures_enabled) {
        glDisable(GL_TEXTURE_2D);
        g_textures_enabled = false;
        g_current_bound_texture = 0;
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

//-----------------------------------------------------------------
// Props.


LTTranslator::LTTranslator(LTfloat x, LTfloat y, LTfloat z, LTProp *target) {
    LTTranslator::x = x;
    LTTranslator::y = y;
    LTTranslator::z = z;
    LTTranslator::target = target;
    target->retain();
}

LTTranslator::~LTTranslator() {
    target->release();
}

void LTTranslator::draw() {
    ltPushMatrix();
    ltTranslate(x, y, z);
    target->draw();
    ltPopMatrix();
}

LTfloat* LTTranslator::field_ptr(const char *field_name) {
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

LTRotator::LTRotator(LTdegrees angle, LTfloat rx, LTfloat ry, LTfloat rz, LTProp *target) {
    LTRotator::angle = angle;
    LTRotator::rx = rx;
    LTRotator::ry = ry;
    LTRotator::rz = rz;
    LTRotator::target = target;
    target->retain();
}

LTRotator::~LTRotator() {
    target->release();
}

void LTRotator::draw() {
    ltPushMatrix();
    ltRotate(angle, rx, ry, rz);
    target->draw();
    ltPopMatrix();
}

LTfloat* LTRotator::field_ptr(const char *field_name) {
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

LTScalor::LTScalor(LTfloat sx, LTfloat sy, LTfloat sz, LTProp *target) {
    LTScalor::sx = sx;
    LTScalor::sy = sy;
    LTScalor::sz = sz;
    LTScalor::target = target;
    target->retain();
}

LTScalor::~LTScalor() {
    target->release();
}

void LTScalor::draw() {
    ltPushMatrix();
    ltScale(sx, sy, sz);
    target->draw();
    ltPopMatrix();
}

LTfloat* LTScalor::field_ptr(const char *field_name) {
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

LTTinter::LTTinter(LTfloat r, LTfloat g, LTfloat b, LTfloat a, LTProp *target) {
    LTTinter::r = r;
    LTTinter::g = g;
    LTTinter::b = b;
    LTTinter::a = a;
    LTTinter::target = target;
    target->retain();
}

LTTinter::~LTTinter() {
    target->release();
}

void LTTinter::draw() {
    LTfloat r0, g0, b0, a0;
    r0 = g_red;
    g0 = g_green;
    b0 = g_blue;
    a0 = g_alpha;
    g_red *= r;
    g_green *= g;
    g_blue *= b;
    g_alpha *= a;
    ltSetColor(g_red, g_green, g_blue, g_alpha);
    target->draw();
    g_red = r0;
    g_green = g0;
    g_blue = b0;
    g_alpha = a0;
    ltSetColor(g_red, g_green, g_blue, g_alpha);
}

LTfloat* LTTinter::field_ptr(const char *field_name) {
    if (strcmp(field_name, "r") == 0) {
        return &r;
    }
    if (strcmp(field_name, "g") == 0) {
        return &g;
    }
    if (strcmp(field_name, "b") == 0) {
        return &b;
    }
    if (strcmp(field_name, "a") == 0) {
        return &a;
    }
    return target->field_ptr(field_name);
}

LTScene::LTScene() {
}

LTScene::~LTScene() {
    std::multimap<LTfloat, LTProp*>::iterator it;
    for (it = scene.begin(); it != scene.end(); it++) {
        ((*it).second)->release();
    }
}

void LTScene::insert(LTProp *prop, LTfloat depth) {
    std::pair<LTfloat, LTProp*> val(depth, prop);
    prop_index[prop] = scene.insert(val);
    prop->retain();
}

void LTScene::remove(LTProp *prop) {
    std::map<LTProp*, std::multimap<LTfloat, LTProp*>::iterator>::iterator pos;
    pos = prop_index.find(prop);
    if (pos != prop_index.end()) {
        prop_index.erase(pos);
        scene.erase((*pos).second);
        prop->release();
    }
}

void LTScene::draw() {
    std::multimap<LTfloat, LTProp*>::iterator it;
    for (it = scene.begin(); it != scene.end(); it++) {
        ((*it).second)->draw();
    }
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

LTImageBuffer *ltReadImage(const char *file) {
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
        PNG_TRANSFORM_GRAY_TO_RGB | PNG_TRANSFORM_SWAP_ALPHA | PNG_TRANSFORM_BGR;
    png_set_filler(png_ptr, 0xFF, PNG_FILLER_BEFORE);
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
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_SWAP_ALPHA | PNG_TRANSFORM_BGR, NULL);

    // Free libpng data.
    png_destroy_write_struct(&png_ptr, &info_ptr);
    delete[] rows;

    fclose(out);
}

void ltPasteImage(LTImageBuffer *src, LTImageBuffer *dest, int x, int y, bool rotate) {
    int src_width;
    int src_height;
    src_width = src->bb_width();
    src_height = src->bb_height();
    int dest_width = dest->bb_width();
    int dest_height = dest->bb_height();
    if (!rotate && (x + src_width > dest_width)) {
        ltAbort("%s too wide to be pasted into %s at x = %d.", 
            src->file, dest->file, x);
    }
    if (!rotate && (y + src_height > dest_height)) {
        ltAbort("%s too high to be pasted into %s at y = %d.",
            src->file, dest->file, y);
    }
    if (rotate && (x + src_height > dest_width)) {
        ltAbort("%s too high to be pasted into %s at x = %d after rotation.",
            src->file, dest->file, x);
    }
    if (rotate && (y + src_width > dest_height)) {
        ltAbort("%s too wide to be pasted into %s at y = %d after rotation.",
            src->file, dest->file, y);
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

//-----------------------------------------------------------------

LTImagePacker::LTImagePacker(int l, int b, int w, int h) {
    left = l;
    bottom = b;
    width = w;
    height = h;
    occupant = NULL;
    rotated = false;
    hi_child = NULL;
    lo_child = NULL;
}

LTImagePacker::~LTImagePacker() {
    if (occupant != NULL) {
        delete hi_child;
        delete lo_child;
    }
}

static bool pack_image(LTImagePacker *packer, LTImageBuffer *img) {
    int pkr_w = packer->width;
    int pkr_h = packer->height;
    int img_w = img->bb_width() + 1; // add 1 pixel buffer.
    int img_h = img->bb_height() + 1;
    if (packer->occupant == NULL) {
        bool fits_rotated = img_h <= pkr_w && img_w <= pkr_h;
        bool fits_non_rotated = img_w <= pkr_w && img_h <= pkr_h;

        if (!fits_rotated && !fits_non_rotated) {
            return false;
        }

        bool should_rotate = false;
        if (!fits_non_rotated) {
            should_rotate = true;
        }

        int hi_l;
        int hi_b;
        int hi_w;
        int hi_h;
        int lo_l;
        int lo_b;
        int lo_w;
        int lo_h;

        if (should_rotate) {
            hi_l = packer->left;
            hi_b = packer->bottom + img_w;
            hi_w = pkr_w;
            hi_h = pkr_h - img_w;
            lo_l = packer->left + img_h;
            lo_b = packer->bottom;
            lo_w = pkr_w - img_h;
            lo_h = img_w;
        } else {
            hi_l = packer->left;
            hi_b = packer->bottom + img_h;
            hi_w = pkr_w;
            hi_h = pkr_h - img_h;
            lo_l = packer->left + img_w;
            lo_b = packer->bottom;
            lo_w = pkr_w - img_w;
            lo_h = img_h;
        }

        packer->occupant = img;
        packer->rotated = should_rotate;
        packer->hi_child = new LTImagePacker(hi_l, hi_b, hi_w, hi_h);
        packer->lo_child = new LTImagePacker(lo_l, lo_b, lo_w, lo_h);

        return true;
    }

    return pack_image(packer->lo_child, img) || pack_image(packer->hi_child, img);
}

static int compare_img_bufs(const void *v1, const void *v2) {
    LTImageBuffer **img1 = (LTImageBuffer **)v1;
    LTImageBuffer **img2 = (LTImageBuffer **)v2;
    int h1 = (*img1)->bb_height();
    int h2 = (*img2)->bb_height();
    if (h1 < h2) {
        return -1;
    } else if (h1 == h2) {
        return 0;
    } else {
        return 1;
    }
}

bool ltPackImage(LTImagePacker *packer, LTImageBuffer *img) {
    if (pack_image(packer, img)) {
        return true;
    } else {
        // Sort images and try again.
        int n = packer->size() + 1;
        bool fitted = true;
        LTImagePacker *test_packer = new LTImagePacker(packer->left, packer->bottom,
            packer->width, packer->height);
        LTImageBuffer **imgs = new LTImageBuffer *[n];
        packer->getImages(imgs);
        imgs[n - 1] = img;
        qsort(imgs, n, sizeof(LTImageBuffer *), compare_img_bufs);
        for (int i = n - 1; i >= 0; i--) {
            if (!pack_image(test_packer, imgs[i])) {
                fitted = false;
                break;
            }
        }
        if (fitted) {
            packer->clear();
            for (int i = n - 1; i >= 0; i--) {
                pack_image(packer, imgs[i]);
            }
        }
        delete test_packer;
        delete[] imgs;
        return fitted;
    }
}

void LTImagePacker::deleteOccupants() {
    if (occupant != NULL) {
        delete occupant;
        occupant = NULL;
        hi_child->deleteOccupants();
        lo_child->deleteOccupants();
        delete hi_child;
        hi_child = NULL;
        delete lo_child;
        lo_child = NULL;
    }
}

void LTImagePacker::clear() {
    if (occupant != NULL) {
        occupant = NULL;
        hi_child->clear();
        lo_child->clear();
        delete hi_child;
        hi_child = NULL;
        delete lo_child;
        lo_child = NULL;
    }
}

int LTImagePacker::size() {
    if (occupant != NULL) {
        return hi_child->size() + lo_child->size() + 1;
    } else {
        return 0;
    }
}

static void get_images(LTImagePacker *packer, int *i, LTImageBuffer **imgs) {
    if (packer->occupant != NULL) {
        imgs[*i] = packer->occupant;
        *i = *i + 1;
        get_images(packer->hi_child, i, imgs);
        get_images(packer->lo_child, i, imgs);
    }
}

void LTImagePacker::getImages(LTImageBuffer **imgs) {
    int i = 0;
    get_images(this, &i, imgs);
}

static void paste_packer_images(LTImageBuffer *img, LTImagePacker *packer) {
    if (packer->occupant != NULL) {
        ltPasteImage(packer->occupant, img, packer->left, packer->bottom, packer->rotated);
        paste_packer_images(img, packer->lo_child);
        paste_packer_images(img, packer->hi_child);
    }
}

//-----------------------------------------------------------------

LTImageBuffer *ltCreateAtlasImage(const char *file, LTImagePacker *packer) {
    int num_pixels = packer->width * packer->height;
    LTImageBuffer *atlas = new LTImageBuffer();
    atlas->width = packer->width;
    atlas->height = packer->height;
    atlas->bb_left = 0;
    atlas->bb_right = packer->width - 1;
    atlas->bb_top = packer->height - 1;
    atlas->bb_bottom = 0;
    atlas->bb_pixels = new LTpixel[num_pixels];
    memset(atlas->bb_pixels, 0x00, num_pixels * 4);
    atlas->file = file;
    paste_packer_images(atlas, packer);
    return atlas;
}

LTtexture ltCreateAtlasTexture(LTImagePacker *packer, const char *dump_file) {
    LTImageBuffer *buf = ltCreateAtlasImage("<internal>", packer);
    if (dump_file != NULL) {
        ltLog("Dumping %s (%d x %d)", dump_file, buf->bb_width(), buf->bb_height());
        ltWriteImage(dump_file, buf);
    }
    LTtexture tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, buf->width, buf->height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buf->bb_pixels);
    delete buf;
    return tex;
}

void ltDeleteTexture(LTtexture tex) {
    glDeleteTextures(1, &tex);
}

//-----------------------------------------------------------------

LTImage::LTImage(LTtexture atls, int atlas_w, int atlas_h, LTImagePacker *packer) {
    if (packer->occupant == NULL) {
        ltAbort("Packer occupant is NULL.");
    }

    atlas = atls;
    rotated = packer->rotated;

    LTfloat fatlas_w = (LTfloat)atlas_w;
    LTfloat fatlas_h = (LTfloat)atlas_h;

    tex_left = (LTfloat)packer->left / fatlas_w;
    tex_bottom = (LTfloat)packer->bottom / fatlas_h;

    bb_left = (LTfloat)packer->occupant->bb_left / fatlas_w;
    bb_bottom = (LTfloat)packer->occupant->bb_bottom / fatlas_h;
    bb_width = (LTfloat)packer->occupant->bb_width() / fatlas_w;
    bb_height = (LTfloat)packer->occupant->bb_height() / fatlas_h;
    orig_width = (LTfloat)packer->occupant->width / fatlas_w;
    orig_height = (LTfloat)packer->occupant->height / fatlas_h;
    pixel_width = packer->occupant->width;
    pixel_height = packer->occupant->height;

    glGenBuffers(1, &vertbuf);
    glBindBuffer(GL_ARRAY_BUFFER, vertbuf);
    setAnchor(LT_ANCHOR_CENTER);

    GLfloat tex_coords[8];
    if (rotated) {
        tex_coords[0] = tex_left + bb_height;   tex_coords[1] = tex_bottom + bb_width;
        tex_coords[2] = tex_left + bb_height;   tex_coords[3] = tex_bottom;
        tex_coords[4] = tex_left;               tex_coords[5] = tex_bottom;
        tex_coords[6] = tex_left;               tex_coords[7] = tex_bottom + bb_width;
    } else {
        tex_coords[0] = tex_left;               tex_coords[1] = tex_bottom + bb_height;
        tex_coords[2] = tex_left + bb_width;    tex_coords[3] = tex_bottom + bb_height;
        tex_coords[4] = tex_left + bb_width;    tex_coords[5] = tex_bottom;
        tex_coords[6] = tex_left;               tex_coords[7] = tex_bottom;
    }
    glGenBuffers(1, &texbuf);
    glBindBuffer(GL_ARRAY_BUFFER, texbuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, tex_coords, GL_STATIC_DRAW);
}

LTImage::~LTImage() {
    glDeleteBuffers(1, &vertbuf);
    glDeleteBuffers(1, &texbuf);
}

void LTImage::draw() {
    ltEnableTexture(atlas);
    glBindBuffer(GL_ARRAY_BUFFER, vertbuf);
    glVertexPointer(2, GL_FLOAT, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, texbuf);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void LTImage::setAnchor(LTAnchor anchor) {
    GLfloat *vertices = NULL;
    switch (anchor) {
        case LT_ANCHOR_CENTER: {
            LTfloat w2 = orig_width * 0.5f;
            LTfloat h2 = orig_width * 0.5f;
            GLfloat v[] = {
                bb_left - w2,               bb_bottom + bb_height - h2,
                bb_left + bb_width - w2,    bb_bottom + bb_height - h2,
                bb_left + bb_width - w2,    bb_bottom - h2,
                bb_left - w2,               bb_bottom - h2
            };
            vertices = v;
            break;
        }
        case LT_ANCHOR_BOTTOM_LEFT: {
            GLfloat v[] = {
                bb_left,                bb_bottom + bb_height,
                bb_left + bb_width,     bb_bottom + bb_height,
                bb_left + bb_width,     bb_bottom,
                bb_left,                bb_bottom
            };
            vertices = v;
            break;
        }
    }
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, vertices, GL_STATIC_DRAW);
}
