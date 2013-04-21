/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltvector)

LTVector::LTVector(int capacity, int stride) {
    LTVector::stride = stride;
    LTVector::capacity = capacity;
    LTVector::size = 0;
    data = new LTfloat[capacity * stride]();
}

LTVector::~LTVector() {
    delete[] data;
}

LT_REGISTER_TYPE(LTVector, "lt.VectorImpl", "lt.Object")

LTDrawVector::LTDrawVector() {
    mode = LT_DRAWMODE_TRIANGLES;
    dimensions = 2;
    vertex_offset = 1;
    color_offset = 0;
    texture_offset = 0;
}

void LTDrawVector::init(lua_State *L) {
    LTSceneNode::init(L);
    vertex_offset--;
    color_offset--;
    texture_offset--;
    if (vector == NULL) {
        luaL_error(L, "A vector must be specified");
    }
    if (dimensions != 2 && dimensions != 3) {
        luaL_error(L, "Dimensions must be 2 or 3 (%d)", dimensions);
    }
    if (color_offset != -1 && color_offset > (vector->stride - 4)) {
        luaL_error(L, "Invalid color offset (%d)", color_offset);
    }
    if (texture_offset != -1 && texture_offset > (vector->stride - 2)) {
        luaL_error(L, "Invalid texture offset (%d)", texture_offset);
    }
    // If there is a texture offset, an image should also be provided.
    if (texture_offset != -1 && image == NULL) {
        luaL_error(L, "An image must be provided if a texture offset is given.");
    }
}

void LTDrawVector::draw() {
    int stride = vector->stride * sizeof(LTfloat);
    ltBindVertBuffer(0);
    ltVertexPointer(dimensions, LT_VERT_DATA_TYPE_FLOAT, stride, vector->data + vertex_offset);
    if (texture_offset >= 0 && image != NULL) {
        ltEnableTexture(image->texture_id);
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

static const LTEnumConstant DrawMode_enum_vals[] = {
    {"triangles",       LT_DRAWMODE_TRIANGLES},
    {"triangle_strip",  LT_DRAWMODE_TRIANGLE_STRIP},
    {"triangle_fan",    LT_DRAWMODE_TRIANGLE_FAN},
    {"points",          LT_DRAWMODE_POINTS},
    {"lines",           LT_DRAWMODE_LINES},
    {"line_strip",      LT_DRAWMODE_LINE_STRIP},
    {NULL, 0}
};
LT_REGISTER_TYPE(LTDrawVector, "lt.DrawVector", "lt.SceneNode")
LT_REGISTER_FIELD_OBJ(LTDrawVector, vector, LTVector)
LT_REGISTER_FIELD_ENUM(LTDrawVector, mode, LTDrawMode, DrawMode_enum_vals)
LT_REGISTER_FIELD_INT(LTDrawVector, dimensions)
LT_REGISTER_FIELD_INT(LTDrawVector, color_offset)
LT_REGISTER_FIELD_INT(LTDrawVector, texture_offset)
LT_REGISTER_FIELD_OBJ(LTDrawVector, image, LTImage)

//void LTDrawTexturedQuads::init(lua_State *L) {
//    LTSceneNode::init(L);
//
//    if (vector->stride - offset < 4) {
//        return luaL_error(L, "Not enough columns (need 4)");
//    }
//
//    if (vector->size <= 0) {
//        num_elems = 0;
//        elements = NULL;
//        return;
//    }
//    num_elems = vector->size + (vector->size >> 1) - 2;
//    elements = new unsigned short[num_elems];
//    unsigned short i = 0;
//    unsigned short j = 0;
//    while (true) {
//        elements[i] = j;
//        elements[i + 1] = j + 1;
//        elements[i + 2] = j + 2;
//        elements[i + 3] = j + 3;
//        if ((i + 4) == num_elems) {
//            break;
//        }
//        // Add degenerate triangles to link quads.
//        elements[i + 4] = j + 3;
//        elements[i + 5] = j + 4;
//        j += 4;
//        i += 6;
//    }
//}
//
//LTDrawTexturedQuads::~LTDrawTexturedQuads() {
//    if (elements != NULL) {
//        delete[] elements;
//    }
//}
//
//void LTDrawTexturedQuads::draw() {
//    if (num_elems == 0) {
//        return;
//    }
//    ltBindVertBuffer(0);
//    int stride = vector->stride * sizeof(LTfloat);
//    ltVertexPointer(2, LT_VERT_DATA_TYPE_FLOAT, stride, vector->data + offset);
//    ltEnableTexture(image->texture_id);
//    ltTexCoordPointer(2, LT_VERT_DATA_TYPE_FLOAT, stride, vector->data + offset + 2);
//    ltDrawElements(LT_DRAWMODE_TRIANGLE_STRIP, num_elems, elements);
//}
