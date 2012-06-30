#include "lt.h"

LT_INIT_IMPL(ltmesh);

LTMesh::LTMesh(int dims, bool has_col, bool has_norm, bool has_tex_coords, LTImage *tex, LTDrawMode mode, void* dat, int sz) {
    dimensions = dims;
    has_colors = has_col;
    has_normals = has_norm;
    has_texture_coords = has_tex_coords;
    texture = tex;
    draw_mode = mode;
    data = dat;
    size = sz;
    dirty = true;
}

LTMesh::~LTMesh() {
    free(data);
    //ltDeleteVertBuffer(vertbuf);
}

void LTMesh::setup() {
    stride = dimensions * 4; // 4 bytes per vertex coord
    stride += has_colors ? 4 : 0; // 1 byte per channel (r, g, b, a = 4 total)
    stride += has_normals ? 4 : 0; // 1 byte per normal component + 1 extra to keep 4-byte alignment
    stride += has_texture_coords ? 4 : 0; // 2 bytes per texture coordinate (u + v) = 4 total
    vertbuf = ltGenVertBuffer();
}

void LTMesh::draw() {
    ensure_buffer_uptodate();
    ltBindVertBuffer(0);
    int offset = 0;
    ltVertexPointer(dimensions, LT_VERT_DATA_TYPE_FLOAT, stride, ((char*)(data)) + offset);
    offset += dimensions * 4;
    if (has_colors) {
        ltEnableColorArrays();
        ltColorPointer(4, LT_VERT_DATA_TYPE_UBYTE, stride, ((char*)(data)) + offset);
        offset += 4;
    }
    if (has_normals) {
        ltEnableNormalArrays();
        ltNormalPointer(LT_VERT_DATA_TYPE_BYTE, stride, ((char*)(data)) + offset);
        offset += 4;
    }
    if (texture != NULL) {
        ltEnableTexture(texture->texture_id);
        ltTexCoordPointer(2, LT_VERT_DATA_TYPE_SHORT, stride, ((char*)(data)) + offset);
    } else {
        ltDisableTextures();
    }
    offset += (has_texture_coords ? 4 : 0);
    ltDrawArrays(draw_mode, 0, size);
    if (has_normals) {
        ltDisableNormalArrays();
    }
    if (has_colors) {
        ltDisableColorArrays();
        ltColorPointer(4, LT_VERT_DATA_TYPE_FLOAT, 0, 0); // XXX necessary?
        ltRestoreTint();
    }
}

void LTMesh::ensure_buffer_uptodate() {
    if (dirty) {
        //ltStaticVertBufferData(size * stride, data);
        dirty = false;
    }
}

void LTMesh::print() {
    printf("     X     Y     Z");
    if (has_colors) {
        printf("  R  G  B  A");
    }
    if (has_normals) {
        printf("    NX    NY    NZ");
    }
    if (has_texture_coords) {
        printf("     U     V");
    }
    printf("\n");
    void *ptr = data;
    for (int i = 0; i < size; i++) {
        LTfloat *vptr = (LTfloat*)ptr;
        printf(" %5.2f %5.2f %5.2f", (double)vptr[0], (double)vptr[1], (double)vptr[2]);
        lt_incr_ptr(ptr, 3 * 4);
        if (has_colors) {
            LTubyte *cptr = (LTubyte*)ptr;
            printf(" %2X %2X %2X %2X", cptr[0], cptr[1], cptr[2], cptr[3]);
            lt_incr_ptr(ptr, 4);
        }
        if (has_normals) {
            LTbyte *nptr = (LTbyte*)ptr;
            printf(" %5.2f %5.2f %5.2f", ((double)nptr[0])/127.0f, ((double)nptr[1])/127.0f, ((double)nptr[2])/127.0f);
            lt_incr_ptr(ptr, 4);
        }
        if (has_texture_coords) {
            LTshort *tptr = (LTshort*)ptr;
            printf(" %5.2f %5.2f", (double)tptr[0]/(double)LT_MAX_TEX_COORD, (double)tptr[1]/(double)LT_MAX_TEX_COORD);
            lt_incr_ptr(ptr, 4);
        }
        printf("\n");
    }
}

LT_REGISTER_TYPE(LTMesh, "lt.Mesh", "lt.SceneNode")
