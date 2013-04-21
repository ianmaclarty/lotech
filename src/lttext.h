/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(lttext)

// 1 byte = 1 glyph
#define LT_MAX_GLYPHS 256

/*
struct LTFont : LTObject {
    LTfloat gap; // Gap between glyphs.
    LTfloat space;
    LTImage *glyphs[LT_MAX_GLYPHS];
};

enum LTTextJustification {
    LT_TEXT_LEFT,
};

struct LTText : LTSceneNode {
   LTFont *font;
   LTTextJustification justification;
   const char *text;
};
*/

std::list<LTImageBuffer *> *ltImageBufferToGlyphs(LTImageBuffer *buf, const char *glyph_chars);
