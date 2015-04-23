/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(lttext)

static bool transparent_column(LTImageBuffer *buf, int col) {
    int w = buf->bb_width();
    int h = buf->bb_height();
    LTpixel *ptr = buf->bb_pixels + col;
    LTpixel *end = buf->bb_pixels + w * h;
    while (ptr < end) {
        if (LT_PIXEL_VISIBLE(*ptr)) {
            return false;
        }
        ptr += w;
    }
    return true;
}

static LTImageBuffer *create_glyph(LTImageBuffer *buf, int start_col, int end_col, char *glyph_str) {
    LTImageBuffer *glyph = new LTImageBuffer(buf->name);
    glyph->scaling = buf->scaling;
    int w = end_col - start_col + 1;
    int h = buf->bb_height();
    int buf_w = buf->bb_width();
    glyph->width = w;
    glyph->height = h;
    glyph->bb_left = 0;
    glyph->bb_top = h - 1;
    glyph->bb_right = w - 1;
    glyph->bb_bottom = 0;
    glyph->is_glyph = true;
    strcpy(glyph->glyph_str, glyph_str);
    LTpixel *pxls = new LTpixel[w * h];
    glyph->bb_pixels = pxls;
    LTpixel *src_ptr = buf->bb_pixels + start_col;
    LTpixel *dest_ptr = pxls;
    LTpixel *end = buf->bb_pixels + buf_w * h;
    while (src_ptr < end) {
        memcpy(dest_ptr, src_ptr, w * 4);
        src_ptr += buf_w;
        dest_ptr += w;
    }
    return glyph;
}

std::list<LTImageBuffer *> *ltImageBufferToGlyphs(LTImageBuffer *buf, const char *glyph_chars) {
    std::list<LTImageBuffer *> *glyphs = new std::list<LTImageBuffer *>();
    int col = 0;
    int glyph_start = 0;
    int glyph_end = 0;
    int num_cols = buf->bb_width();
    unsigned char *chr = (unsigned char*)glyph_chars;
    unsigned char str[10];
    while (*chr != 0 && col < num_cols) {
        while (col < num_cols && !transparent_column(buf, col)) {
            col++;
        }
        glyph_end = col - 1;
        if (*chr < 128) {
            str[0] = *chr;
            str[1] = 0;
        } else {
            memset(str, 0, 10);
            str[0] = *chr;
            chr++;
            int i = 1;
            while (i < 10 && *chr != 0 && (*chr >> 6) == 2) {
                str[i++] = *chr;
                chr++;
            }
            chr--;
        }
        LTImageBuffer *glyph = create_glyph(buf, glyph_start, glyph_end, (char*)str);
        glyphs->push_back(glyph);
        while (col < num_cols && transparent_column(buf, col)) {
            col++;
        }
        glyph_start = col - 1;
        chr++;
    }
    return glyphs;
}
