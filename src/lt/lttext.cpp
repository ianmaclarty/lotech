/* Copyright (C) 2010 Ian MacLarty */
#include "lttext.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool transparent_column(LTImageBuffer *buf, int col) {
    int w = buf->bb_width();
    int h = buf->bb_height();
    LTpixel *ptr = buf->bb_pixels + col;
    LTpixel *end = buf->bb_pixels + w * h;
    while (ptr < end) {
        if (!LT_TRANSPARENT(*ptr)) {
            return false;
        }
        ptr += w;
    }
    return true;
}

static LTImageBuffer *create_glyph(LTImageBuffer *buf, int start_col, int end_col, char chr) {
    LTImageBuffer *glyph = new LTImageBuffer(buf->filename);
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
    glyph->glyph_char = chr;
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
    const char *chr = glyph_chars;
    while (*chr != '\0' && col < num_cols) {
        while (col < num_cols && !transparent_column(buf, col)) {
            col++;
        }
        glyph_end = col - 1;
        LTImageBuffer *glyph = create_glyph(buf, glyph_start, glyph_end, *chr);
        glyphs->push_back(glyph);
        while (col < num_cols && transparent_column(buf, col)) {
            col++;
        }
        glyph_start = col - 1;
        chr++;
    }
    return glyphs;
}
