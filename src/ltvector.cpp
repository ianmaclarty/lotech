/* Copyright (C) 2010 Ian MacLarty */

#include "ltvector.h"

LTVector::LTVector(int capacity, int stride) : LTObject(LT_TYPE_VECTOR) {
    LTVector::stride = stride;
    LTVector::capacity = capacity;
    LTVector::size = 0;
    data = new LTfloat[capacity * stride]();
}

LTVector::~LTVector() {
    delete[] data;
}

LTDrawVector::LTDrawVector(LTDrawMode mode, LTVector *vector,
    int dims, int vertex_os, int color_os, int tex_os, LTImage *img) : LTSceneNode(LT_TYPE_DRAWVECTOR)
{
    LTDrawVector::mode = mode;
    LTDrawVector::vector = vector;
    LTDrawVector::dimensions = dims;
    LTDrawVector::vertex_offset = vertex_os;
    LTDrawVector::color_offset = color_os;
    LTDrawVector::texture_offset = tex_os;
    LTDrawVector::image = img;
}

void LTDrawVector::draw() {
    int stride = vector->stride * sizeof(LTfloat);
    ltBindVertBuffer(0);
    ltVertexPointer(dimensions, LT_VERT_DATA_TYPE_FLOAT, stride, vector->data + vertex_offset);
    if (texture_offset >= 0 && image != NULL) {
        ltEnableAtlas(image->atlas);
        ltTexCoordPointer(2, LT_VERT_DATA_TYPE_FLOAT, stride, vector->data + texture_offset);
    } else {
        ltDisableTextures();
    }
    if (color_offset >= 0) {
        ltEnableColorArrays();
        ltColorPointer(4, LT_VERT_DATA_TYPE_FLOAT, stride, vector->data + color_offset);
    }
    ltDrawArrays(mode, 0, vector->size);
    if (color_offset >= 0) {
        ltDisableColorArrays();
        ltColorPointer(4, LT_VERT_DATA_TYPE_FLOAT, 0, 0); // XXX necessary?
        ltRestoreTint();
    }
}

LTDrawTexturedQuads::LTDrawTexturedQuads(LTVector *vector,
    int offset, LTImage *img) : LTSceneNode(LT_TYPE_DRAWQUADS)
{
    LTDrawTexturedQuads::vector = vector;
    LTDrawTexturedQuads::offset = offset;
    LTDrawTexturedQuads::image = img;

    if (vector->size <= 0) {
        num_elems = 0;
        elements = NULL;
        return;
    }
    num_elems = vector->size + (vector->size >> 1) - 2;
    elements = new unsigned short[num_elems];
    unsigned short i = 0;
    unsigned short j = 0;
    while (true) {
        elements[i] = j;
        elements[i + 1] = j + 1;
        elements[i + 2] = j + 2;
        elements[i + 3] = j + 3;
        if ((i + 4) == num_elems) {
            break;
        }
        // Add degenerate triangles to link quads.
        elements[i + 4] = j + 3;
        elements[i + 5] = j + 4;
        j += 4;
        i += 6;
    }
}

LTDrawTexturedQuads::~LTDrawTexturedQuads() {
    if (elements != NULL) {
        delete[] elements;
    }
}

void LTDrawTexturedQuads::draw() {
    if (num_elems == 0) {
        return;
    }
    ltBindVertBuffer(0);
    int stride = vector->stride * sizeof(LTfloat);
    ltVertexPointer(2, LT_VERT_DATA_TYPE_FLOAT, stride, vector->data + offset);
    ltEnableAtlas(image->atlas);
    ltTexCoordPointer(2, LT_VERT_DATA_TYPE_FLOAT, stride, vector->data + offset + 2);
    ltDrawElements(LT_DRAWMODE_TRIANGLE_STRIP, num_elems, elements);
}
