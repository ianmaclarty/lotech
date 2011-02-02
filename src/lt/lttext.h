/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTTEXT_H
#define LTTEXT_H

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
#include "ltscene.h"
#include "ltgraphics.h"
#include "ltimage.h"

// 1 byte = 1 glyph
#define LT_MAX_GLYPHS 256

/*
struct LTFont : LTObject {
    LTImage *glyphs[LT_MAX_GLYPHS];
    LTfloat space;
    LTfloat base;
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

std::list<LTImageBuffer *> *ltImageBufferToGlyphs(LTImageBuffer *buf);

#endif
