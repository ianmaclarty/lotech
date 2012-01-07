/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTTEXT_H
#define LTTEXT_H

#include <list>
#include <map>

#include <string.h>

#include "ltcommon.h"
#include "ltevent.h"
#include "ltscene.h"
#include "ltgraphics.h"
#include "ltimage.h"

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

#endif
