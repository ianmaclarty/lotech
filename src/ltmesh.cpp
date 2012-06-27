#include "lt.h"

LT_INIT_IMPL(ltmesh);

LTMesh::LTMesh(int dims, bool has_col, bool has_norm, LTImage *tex, LTDrawMode mode, void* dat, int sz) {
    dimensions = dims;
    has_color = has_col;
    has_normals = has_norm;
    texture = tex;
    draw_mode = mode;
    data = dat;
    size = sz;
    setup();
}

LTMesh::~LTMesh() {
    free(data);
    ltDeleteVertBuffer(vertbuf);
}

void LTMesh::setup() {
    stride = dimensions * 4; // 4 bytes per vertex coord
    stride += has_color ? 4 : 0; // 1 byte per channel (r, g, b, a = 4 total)
    stride += has_normals ? 4 : 0; // 1 byte per normal component + 1 extra to keep 4-byte alignment
    stride += texture != NULL ? 4 : 0; // 2 bytes per texture coordinate (u + v) = 4 total
    vertbuf = ltGenVertBuffer();
    dirty = true;
}

void LTMesh::draw() {
    ensure_buffer_uptodate();
    ltBindVertBuffer(vertbuf);
    int offset = 0;
    ltVertexPointer(dimensions, LT_VERT_DATA_TYPE_FLOAT, stride, (void*)offset);
    offset += dimensions * 4;
    if (has_color) {
        ltEnableColorArrays();
        ltColorPointer(4, LT_VERT_DATA_TYPE_BYTE, stride, (void*)offset);
        offset += 4;
    }
    if (has_normals) {
        ltEnableNormalArrays();
        ltNormalPointer(LT_VERT_DATA_TYPE_BYTE, stride, (void*)offset);
        offset += 4;
    }
    if (texture != NULL) {
        ltEnableTexture(texture->texture_id);
        ltTexCoordPointer(2, LT_VERT_DATA_TYPE_SHORT, stride, (void*)offset);
        offset += 4;
    } else {
        ltDisableTextures();
    }
    ltDrawArrays(draw_mode, 0, size);
    if (has_normals) {
        ltDisableNormalArrays();
    }
    if (has_color) {
        ltDisableColorArrays();
        ltColorPointer(4, LT_VERT_DATA_TYPE_FLOAT, 0, 0); // XXX necessary?
        ltRestoreTint();
    }
}

void LTMesh::ensure_buffer_uptodate() {
    if (dirty) {
        ltStaticVertBufferData(size * stride, data);
        dirty = false;
    }
}

LT_REGISTER_TYPE(LTMesh, "lt.Mesh", "lt.SceneNode")
