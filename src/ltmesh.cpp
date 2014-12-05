/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltmesh);

LTMesh::LTMesh() {
    dimensions = 2;
    has_colors = false;
    has_normals = false;
    has_texture_coords = false;
    texture = NULL;
    texture_ref = LUA_NOREF;
    draw_mode = LT_DRAWMODE_TRIANGLES;
    vdata = NULL;
    size = 0;
    vertbuf = 0;
    vb_dirty = false;
    left = 0.0f;
    right = 0.0f;
    bottom = 0.0f;
    top = 0.0f;
    farz = 0.0f;
    nearz = 0.0f;
    bb_dirty = false;
    indices = NULL;
    num_indices = 0;
    indices_dirty = false;
}

LTMesh::LTMesh(LTMesh *mesh) {
    dimensions = mesh->dimensions;
    has_colors = mesh->has_colors;
    has_normals = mesh->has_normals;
    has_texture_coords = mesh->has_texture_coords;
    texture = mesh->texture;
    texture_ref = LUA_NOREF;
    draw_mode = mesh->draw_mode;

    size = mesh->size;
    if (mesh->vdata != NULL) {
        vdata = new LTVertData[size];
        memcpy(vdata, mesh->vdata, sizeof(LTVertData) * size);
    } else {
        vdata = NULL;
    }
    vb_dirty = true;

    left = mesh->left;
    right = mesh->right;
    bottom = mesh->bottom;
    top = mesh->top;
    farz = mesh->farz;
    nearz = mesh->nearz;
    bb_dirty = mesh->bb_dirty;

    vertbuf = 0;

    if (mesh->num_indices > 0) {
        indices = new LTvertindex[mesh->num_indices];
        memcpy(indices, mesh->indices, mesh->num_indices * sizeof(LTvertindex));
    } else {
        indices = NULL;
    }
    num_indices = mesh->num_indices;
    indices_dirty = num_indices > 0;
}

LTMesh::LTMesh(LTTexturedNode *img) {
    dimensions = 2;
    has_colors = false;
    has_normals = false;
    has_texture_coords = true;
    texture = img;
    texture_ref = LUA_NOREF;
    draw_mode = LT_DRAWMODE_TRIANGLES;

    size = 4;
    vdata = new LTVertData[size];

    for (int i = 0; i < size; i++) {
        vdata[i].xyz.x = img->world_vertices[i*2];
        vdata[i].xyz.y = img->world_vertices[i*2+1];
        vdata[i].uv.u = (LTfloat)img->tex_coords[i*2] / (LTfloat)LT_MAX_TEX_COORD;
        vdata[i].uv.v = (LTfloat)img->tex_coords[i*2+1] / (LTfloat)LT_MAX_TEX_COORD;
    }

    vb_dirty = true;

    num_indices = 6;
    indices = new LTvertindex[num_indices];
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 0;
    indices[4] = 2;
    indices[5] = 3;

    indices_dirty = true;

    left = img->world_vertices[0];
    right = img->world_vertices[2];
    bottom = img->world_vertices[1];
    top = img->world_vertices[5];
    farz = 0;
    nearz = 0;
    bb_dirty = false;

    vertbuf = 0;
}

LTMesh::LTMesh(int dims, bool has_col, bool has_norm, bool has_tex_coords, LTImage *tex, LTDrawMode mode, LTVertData* dat, int sz) {
    dimensions = dims;
    has_colors = has_col;
    has_normals = has_norm;
    has_texture_coords = has_tex_coords;
    texture = tex;
    texture_ref = LUA_NOREF;
    draw_mode = mode;
    vdata = dat;
    size = sz;
    vb_dirty = true;
    bb_dirty = true;

    vertbuf = 0;

    indices = NULL;
    num_indices = 0;
    indices_dirty = false;
}

LTMesh::~LTMesh() {
    if (vdata != NULL) {
        delete[] vdata;
    }
    if (indices != NULL) {
        delete[] indices;
    }
    if (vertbuf != 0) {
        ltDeleteVertBuffer(vertbuf);
    }
}

void LTMesh::resize_data(int sz) {
    if (sz == 0) {
        if (vdata != NULL) delete[] vdata;
        vdata = NULL;
    } else {
        LTVertData *tmp = new LTVertData[sz];
        memcpy(tmp, vdata, size * sizeof(LTVertData));
        delete[] vdata;
        vdata = tmp;
    }
    size = sz;
    vb_dirty = true;
    bb_dirty = true;
}

void LTMesh::resize_indices(int sz) {
    if (sz == 0) {
        if (indices != NULL) delete[] indices;
        indices = NULL;
    } else {
        LTvertindex *tmp = new LTvertindex[sz];
        memcpy(tmp, indices, sizeof(LTvertindex) * num_indices);
        delete[] indices;
        indices = tmp;
    }
    num_indices = sz;
    indices_dirty = true;
}

void LTMesh::compute_normals() {
    if (vdata == NULL) return;

    assert(indices != NULL);
    assert(num_indices % 3 == 0);
    assert(draw_mode == LT_DRAWMODE_TRIANGLES);

    int n = num_indices;

    for (int i = 0; i < size; i++) {
        vdata[i].normal.zero();
    }

    for (int i = 0; i < n; i += 3) {
        LTVertData *vd1 = &vdata[indices[i + 0]];
        LTVertData *vd2 = &vdata[indices[i + 1]];
        LTVertData *vd3 = &vdata[indices[i + 2]];
        LTVec3 *p1 = &vd1->xyz;
        LTVec3 *p2 = &vd2->xyz;
        LTVec3 *p3 = &vd3->xyz;
        LTVec3 v1 = *p2 - *p1;
        LTVec3 v2 = *p3 - *p1;
        LTVec3 nm = v1.cross(v2);
        vd1->normal += nm;
        vd2->normal += nm;
        vd3->normal += nm;
    }

    for (int i = 0; i < size; i++) {
        vdata[i].normal.normalize();
    }

    has_normals = true;

    vb_dirty = true;
}

static inline
int compute_stride(LTMesh *m) {
    int stride;
    stride = m->dimensions * 4; // 4 bytes per vertex coord
    stride += m->has_colors ? 4 : 0; // 1 byte per channel (r, g, b, a = 4 total)
    stride += m->has_normals ? 4 : 0; // 1 byte per normal component + 1 extra to keep 4-byte alignment
    stride += m->has_texture_coords ? 4 : 0; // 2 bytes per texture coordinate (u + v) = 4 total
    return stride;
}

void *LTMesh::generate_vbo_data() {
    int stride = compute_stride(this);
    void* data = malloc(stride * size);
    void* ptr = data;
    for (int i = 0; i < size; i++) {
        LTVertData *vd = &vdata[i];
        *((LTfloat*)ptr + 0) = vd->xyz.x;
        *((LTfloat*)ptr + 1) = vd->xyz.y;
        lt_incr_ptr(&ptr, 8);
        if (dimensions > 2) {
            *((LTfloat*)ptr) = vd->xyz.z;
            lt_incr_ptr(&ptr, 4);
        }
        if (has_colors) {
            *((LTubyte*)ptr + 0) = (LTubyte)(vd->color.red * 255.0);
            *((LTubyte*)ptr + 1) = (LTubyte)(vd->color.green * 255.0);
            *((LTubyte*)ptr + 2) = (LTubyte)(vd->color.blue * 255.0);
            *((LTubyte*)ptr + 3) = (LTubyte)(vd->color.alpha * 255.0);
            lt_incr_ptr(&ptr, 4);
        }
        if (has_normals) {
            *((LTbyte*)ptr + 0) = (LTbyte)(vd->normal.x * 127.0);
            *((LTbyte*)ptr + 1) = (LTbyte)(vd->normal.y * 127.0);
            *((LTbyte*)ptr + 2) = (LTbyte)(vd->normal.z * 127.0);
            lt_incr_ptr(&ptr, 4);
        }
        if (has_texture_coords) {
            *((LTtexcoord*)ptr + 0) = (LTtexcoord)(vd->uv.u * (LTfloat)LT_MAX_TEX_COORD);
            *((LTtexcoord*)ptr + 1) = (LTtexcoord)(vd->uv.v * (LTfloat)LT_MAX_TEX_COORD);
            lt_incr_ptr(&ptr, 4);
        }
    }
    assert(ptr == (void*)((char*)data + size * stride));
    return data;
}

void LTMesh::draw() {
    ensure_vb_uptodate();
    int stride = compute_stride(this);
    ltBindVertBuffer(vertbuf);
    LTuintptr offset = 0;
    ltVertexPointer(dimensions, LT_VERT_DATA_TYPE_FLOAT, stride, (void*)offset);
    offset += dimensions * 4;
    if (has_colors) {
        ltEnableColorArrays();
        ltColorPointer(4, LT_VERT_DATA_TYPE_UBYTE, stride, (void*)offset);
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
    } else {
        ltDisableTextures();
    }
    if (num_indices > 0) {
        ltDrawElements(draw_mode, num_indices, indices);
    } else {
        ltDrawArrays(draw_mode, 0, size);
    }
    if (has_normals) {
        ltDisableNormalArrays();
    }
    if (has_colors) {
        ltDisableColorArrays();
        ltColorPointer(4, LT_VERT_DATA_TYPE_FLOAT, 0, 0); // XXX necessary?
        ltRestoreTint();
    }
}

void LTMesh::stretch(LTfloat px, LTfloat py, LTfloat pz,
    LTfloat left, LTfloat right, LTfloat down, LTfloat up, LTfloat backward, LTfloat forward)
{
    if (vdata == NULL) return;
    for (int i = 0; i < size; i++) {
        LTVertData *vd = &vdata[i];
        LTVec3 *p = &vd->xyz;
        if (p->x < px) {
            p->x -= left;
        } else {
            p->x += right;
        }
        if (p->y < py) {
            p->y -= down;
        } else {
            p->y += up;
        }
        if (p->z < pz) {
            p->z -= backward;
        } else {
            p->z += forward;
        }
    }

    vb_dirty = true;
    bb_dirty = true;
}

void LTMesh::shift(LTfloat sx, LTfloat sy, LTfloat sz) {
    if (vdata == NULL) return;

    for (int i = 0; i < size; i++) {
        LTVertData *vd = &vdata[i];
        vd->xyz += LTVec3(sz, sy, sz);
    }

    vb_dirty = true;
    bb_dirty = true;
}

void LTMesh::merge(LTMesh *mesh) {
    assert(mesh->dimensions == dimensions);
    assert(mesh->has_colors == has_colors);
    assert(mesh->has_normals == has_normals);
    assert(mesh->has_texture_coords == has_texture_coords);
    assert(mesh->draw_mode == draw_mode);
    assert(draw_mode == LT_DRAWMODE_TRIANGLES); // XXX Need to add degenerated vertices for others?
    assert((mesh->indices == NULL) == (indices == NULL));

    int orig_size = size;

    if (mesh->vdata != NULL) {
        LTVertData *tmp = new LTVertData[size + mesh->size];
        memcpy(tmp, vdata, sizeof(LTVertData) * size);
        memcpy(&tmp[size], mesh->vdata, sizeof(LTVertData) * mesh->size);
        delete[] vdata;
        vdata = tmp;
    }
    size += mesh->size;

    if (mesh->indices != NULL) {
        LTvertindex *tmp = new LTvertindex[num_indices + mesh->num_indices];
        memcpy(tmp, indices, sizeof(LTvertindex) * num_indices);
        for (int i = 0; i < mesh->num_indices; i++) {
            tmp[i + num_indices] = mesh->indices[i] + orig_size;
        }
        delete[] indices;
        indices = tmp;
    }
    num_indices += mesh->num_indices;

    vb_dirty = true;
    bb_dirty = true;
    indices_dirty = true;
}

void LTMesh::grid(int rows, int columns) {
    if (vdata == NULL) return;
    assert(dimensions == 2);
    assert(has_texture_coords);
    assert(!has_colors);
    assert(!has_normals);
    
    if (indices != NULL) {
        delete[] indices;
    }
    num_indices = 6 * rows * columns;
    indices = new LTvertindex[num_indices];

    draw_mode = LT_DRAWMODE_TRIANGLES;

    ensure_bb_uptodate();

    size = (rows + 1) * (columns + 1);
    if (vdata != NULL) {
        delete[] vdata;
    }
    vdata = new LTVertData[size];

    LTfloat col_width = (right - left) / (LTfloat)columns;
    LTfloat row_height = (top - bottom) / (LTfloat)rows;

    LTfloat tex_left = (LTfloat)texture->tex_coords[0] / (LTfloat)LT_MAX_TEX_COORD;
    LTfloat tex_right = (LTfloat)texture->tex_coords[2] / (LTfloat)LT_MAX_TEX_COORD;
    LTfloat tex_bottom = (LTfloat)texture->tex_coords[1] / (LTfloat)LT_MAX_TEX_COORD;
    LTfloat tex_top = (LTfloat)texture->tex_coords[5] / (LTfloat)LT_MAX_TEX_COORD;
    LTfloat tex_col_width = (tex_right - tex_left) / (LTfloat)columns;
    LTfloat tex_row_height = (tex_top - tex_bottom) / (LTfloat)rows;

    LTfloat y = bottom;
    LTfloat v = tex_bottom;

    int i = 0;
    int j = 0;
    for (int row = 0; row <= rows; row++) {
        LTfloat x = left;
        LTfloat u = tex_left;
        for (int col = 0; col <= columns; col++) {
            // Add vertex
            vdata[i].xyz.x = x;
            vdata[i].xyz.y = y;
            vdata[i].uv.u = u;
            vdata[i].uv.v = v;

            if (col < columns && row < rows) {
                // Add indicies
                indices[j + 0] = i;
                indices[j + 1] = i + columns + 1;
                indices[j + 2] = i + 1;
                indices[j + 3] = i + 1;
                indices[j + 4] = i + columns + 1;
                indices[j + 5] = i + columns + 2;
                j += 6;
            }

            i++;
            
            x += col_width;
            u += tex_col_width;
            if (col == columns - 1) {
                // in case of rounding errors.
                x = right;
                u = tex_right;
            }
        }
        y += row_height;
        v += tex_row_height;
        if (row == rows - 1) {
            y = top;
            v = tex_top;
        }
    }
    assert(j == num_indices);
    assert(i == size);

    vb_dirty = true;
    indices_dirty = true;
    bb_dirty = true;
}

void LTMesh::ensure_vb_uptodate() {
    if (vertbuf == 0) {
        vertbuf = ltGenVertBuffer();
    }
    if (vb_dirty) {
        ltBindVertBuffer(vertbuf);
        void *data = generate_vbo_data();
        ltStaticVertBufferData(size * compute_stride(this), data);
        free(data);
        vb_dirty = false;
    }
}

void LTMesh::ensure_bb_uptodate() {
    if (vdata != NULL && bb_dirty) {
        if (size > 0) {
            LTVertData *vd = &vdata[0];
            left = vd->xyz.x;
            right = left;
            bottom = vd->xyz.y;
            top = bottom;
            farz = vd->xyz.z;
            nearz = farz;

            for (int i = 1; i < size; i++) {
                LTVertData *vd = &vdata[i];
                if (vd->xyz.x > right) {
                    right = vd->xyz.x;
                } else if (vd->xyz.x < left) {
                    left = vd->xyz.x;
                }
                if (vd->xyz.y > top) {
                    top = vd->xyz.y;
                } else if (vd->xyz.y < bottom) {
                    bottom = vd->xyz.y;
                }
                if (vd->xyz.z > nearz) {
                    nearz = vd->xyz.z;
                } else if (vd->xyz.z < farz) {
                    farz = vd->xyz.z;
                }
            }
        } else {
            left = 0;
            right = 0;
            bottom = 0;
            top = 0;
            farz = 0;
            nearz = 0;
        }
        bb_dirty = false;
    }
}

void LTMesh::print() {
    if (dimensions == 2) {
        printf("     X     Y");
    } else {
        printf("     X     Y     Z");
    }
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
    if (vdata == NULL) {
        printf("NO DATA\n");
    } else {
        for (int i = 0; i < size; i++) {
            LTVertData *vd = &vdata[i];
            if (dimensions == 2) {
                printf(" %5.2f %5.2f", vd->xyz.x, vd->xyz.y);
            } else {
                printf(" %5.2f %5.2f %5.2f", vd->xyz.x, vd->xyz.y, vd->xyz.z);
            }
            if (has_colors) {
                printf(" %1.2f %1.2f %1.2f %1.2f", vd->color.red, vd->color.green, vd->color.blue,
                    vd->color.alpha);
            }
            if (has_normals) {
                printf(" %5.2f %5.2f %5.2f", vd->normal.x, vd->normal.y, vd->normal.y);
            }
            if (has_texture_coords) {
                printf(" %5.2f %5.2f", vd->uv.u, vd->uv.v);
            }
            printf("\n");
        }
    }
    if (indices != NULL) {
        printf("indices = [");
        for (int i = 0; i < num_indices; i++) {
            printf("%d, ", indices[i]);
        }
        printf("]\n");
    } else {
        printf("NO INDICES\n");
    }
}

static int clone_mesh(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    LTMesh *mesh = lt_expect_LTMesh(L, 1);
    LTMesh *clone = new (lt_alloc_LTMesh(L)) LTMesh(mesh);
    if (mesh->texture != NULL) {
        // Add reference from clone to texture node
        ltLuaGetRef(L, 1, mesh->texture_ref);
        clone->texture_ref = ltLuaAddRef(L, -2, -1);
        lua_pop(L, 1); // pop texture node
    }
    return 1;
}

static int stretch_mesh(lua_State *L) {
    ltLuaCheckNArgs(L, 10);
    LTMesh *mesh = lt_expect_LTMesh(L, 1);
    LTfloat px = luaL_checknumber(L, 2);
    LTfloat py = luaL_checknumber(L, 3);
    LTfloat pz = luaL_checknumber(L, 4);
    LTfloat left = luaL_checknumber(L, 5);
    LTfloat right = luaL_checknumber(L, 6);
    LTfloat down = luaL_checknumber(L, 7);
    LTfloat up = luaL_checknumber(L, 8);
    LTfloat backward = luaL_checknumber(L, 9);
    LTfloat forward = luaL_checknumber(L, 10);
    mesh->stretch(px, py, pz, left, right, down, up, backward, forward);
    lua_pushvalue(L, 1);
    return 1;
}

static int shift_mesh(lua_State *L) {
    ltLuaCheckNArgs(L, 4);
    LTMesh *mesh = lt_expect_LTMesh(L, 1);
    LTfloat sx = luaL_checknumber(L, 2);
    LTfloat sy = luaL_checknumber(L, 3);
    LTfloat sz = luaL_checknumber(L, 4);
    mesh->shift(sx, sy, sz);
    lua_pushvalue(L, 1);
    return 1;
}

static int merge_mesh(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTMesh *mesh1 = lt_expect_LTMesh(L, 1);
    LTMesh *mesh2 = lt_expect_LTMesh(L, 2);

    if (mesh1->dimensions != mesh2->dimensions) {
        return luaL_error(L, "Mesh merge error: incompatible dimensions");
    }
    if (mesh1->has_colors != mesh2->has_colors) {
        return luaL_error(L, "Mesh merge error: one has colors, the other doesn't");
    }
    if (mesh1->has_normals != mesh2->has_normals) {
        return luaL_error(L, "Mesh merge error: one has normals, the other doesn't");
    }
    if (mesh1->has_texture_coords != mesh2->has_texture_coords) {
        return luaL_error(L, "Mesh merge error: one has texture coords, the other doesn't");
    }
    if ((mesh1->indices == NULL) != (mesh2->indices == NULL)) {
        return luaL_error(L, "Mesh merge error: either both or none should have indices");
    }
    if (mesh1->draw_mode != LT_DRAWMODE_TRIANGLES || mesh2->draw_mode != LT_DRAWMODE_TRIANGLES) {
        return luaL_error(L, "Mesh merge error: sorry, only triangle meshes can be merged");
    }
    mesh1->merge(mesh2);
    lua_pushvalue(L, 1);
    return 1;
}

static int make_grid(lua_State *L) {
    ltLuaCheckNArgs(L, 3);
    LTMesh *mesh = lt_expect_LTMesh(L, 1);
    if (mesh->dimensions != 2) {
        return luaL_error(L, "Mesh must be 2D");
    } else if (mesh->has_colors) {
        return luaL_error(L, "Mesh may not have colors");
    } else if (mesh->has_normals) {
        return luaL_error(L, "Mesh may not have normals");
    } else if (!mesh->has_texture_coords) {
        return luaL_error(L, "Mesh must have a texture");
    }
    int rows = luaL_checkinteger(L, 2);
    int columns = luaL_checkinteger(L, 3);
    mesh->grid(rows, columns);
    lua_pushvalue(L, 1);
    return 1;
}

static int set_xys(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTMesh *mesh = lt_expect_LTMesh(L, 1);
    if (!lua_istable(L, 2)) {
        return luaL_error(L, "Expecting a table in argument 2");
    }
    int len = lua_objlen(L, 2);
    if (len & 0x1) {
        return luaL_error(L, "table should have even length (in fact %d)", len);
    }
    int size = len / 2;
    if (size != mesh->size) {
        mesh->resize_data(size);
    }
    LTVertData *ptr = mesh->vdata;
    for (int i = 1; i <= len; i += 2) {
        lua_rawgeti(L, 2, i);
        lua_rawgeti(L, 2, i  + 1);
        LTfloat x = luaL_checknumber(L, -2);
        LTfloat y = luaL_checknumber(L, -1);
        lua_pop(L, 2);
        ptr->xyz.x = x;
        ptr->xyz.y = y;
        ptr++;
    }

    mesh->vb_dirty = true;
    mesh->bb_dirty = true;

    return 0;
}

static int set_xyzs(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTMesh *mesh = lt_expect_LTMesh(L, 1);
    if (!lua_istable(L, 2)) {
        return luaL_error(L, "Expecting a table in argument 2");
    }
    int len = lua_objlen(L, 2);
    if (len % 3 != 0) {
        return luaL_error(L, "table should have length divisible by 3 (in fact %d)", len);
    }
    if (mesh->dimensions != 3) {
        mesh->dimensions = 3;
    }
    int size = len / 3;
    if (size > mesh->size) {
        mesh->resize_data(size);
    }
    LTVertData *ptr = mesh->vdata;
    for (int i = 1; i <= len; i += 3) {
        lua_rawgeti(L, 2, i + 0);
        lua_rawgeti(L, 2, i + 1);
        lua_rawgeti(L, 2, i + 2);
        LTfloat x = luaL_checknumber(L, -3);
        LTfloat y = luaL_checknumber(L, -2);
        LTfloat z = luaL_checknumber(L, -1);
        lua_pop(L, 3);
        ptr->xyz.x = x;
        ptr->xyz.y = y;
        ptr->xyz.z = z;
        ptr++;
    }

    mesh->vb_dirty = true;
    mesh->bb_dirty = true;

    return 0;
}

static int set_rgbs(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTMesh *mesh = lt_expect_LTMesh(L, 1);
    if (!lua_istable(L, 2)) {
        return luaL_error(L, "Expecting a table in argument 2");
    }
    int len = lua_objlen(L, 2);
    if (len % 3 != 0) {
        return luaL_error(L, "table should have length divisible by 3 (in fact %d)", len);
    }
    mesh->has_colors = true;
    int size = len / 3;
    if (size > mesh->size) {
        mesh->resize_data(size);
    }
    LTVertData *ptr = mesh->vdata;
    for (int i = 1; i <= len; i += 3) {
        lua_rawgeti(L, 2, i + 0);
        lua_rawgeti(L, 2, i + 1);
        lua_rawgeti(L, 2, i + 2);
        LTfloat r = luaL_checknumber(L, -3);
        LTfloat g = luaL_checknumber(L, -2);
        LTfloat b = luaL_checknumber(L, -1);
        lua_pop(L, 3);
        ptr->color.red = r;
        ptr->color.green = g;
        ptr->color.blue = b;
        ptr++;
    }

    mesh->vb_dirty = true;
    mesh->bb_dirty = true;

    return 0;
}

static int set_uvs(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTMesh *mesh = lt_expect_LTMesh(L, 1);
    if (!lua_istable(L, 2)) {
        return luaL_error(L, "Expecting a table in argument 2");
    }
    int len = lua_objlen(L, 2);
    if (len & 0x1) {
        return luaL_error(L, "table should have even length (in fact %d)", len);
    }
    int size = len / 2;
    if (size != mesh->size) {
        mesh->resize_data(size);
    }
    LTVertData *ptr = mesh->vdata;
    for (int i = 1; i <= len; i += 2) {
        lua_rawgeti(L, 2, i);
        lua_rawgeti(L, 2, i  + 1);
        LTfloat u = luaL_checknumber(L, -2);
        LTfloat v = luaL_checknumber(L, -1);
        lua_pop(L, 2);
        ptr->uv.u = u;
        ptr->uv.v = v;
        ptr++;
    }

    mesh->vb_dirty = true;
    mesh->bb_dirty = true;
    mesh->has_texture_coords = true;

    return 0;
}

#define MAX_INDICES 65000

static int set_indices(lua_State *L) {
    ltLuaCheckNArgs(L, 2);
    LTMesh *mesh = lt_expect_LTMesh(L, 1);
    if (!lua_istable(L, 2)) {
        return luaL_error(L, "Expecting a table in argument 2");
    }
    int n = lua_objlen(L, 2);
    int size = mesh->size;
    if (n > mesh->num_indices) {
        mesh->resize_indices(n);
    }
    for (int i = 0; i < n; i++) {
        lua_rawgeti(L, 2, i + 1);
        int index = luaL_checkint(L, -1);
        lua_pop(L, 1);
        if (index > size || index < 1 || index > MAX_INDICES) {
            return luaL_error(L, "Invalid index: %d", index);
        }
        mesh->indices[i] = (LTvertindex)(index - 1);
    }

    mesh->indices_dirty = true;

    return 0;
}

static int compute_normals(lua_State *L) {
    LTMesh *mesh = lt_expect_LTMesh(L, 1);
    if (mesh->size == 0) {
        return 0;
    }
    mesh->compute_normals();

    return 0;
}

static int print_mesh(lua_State *L) {
    LTMesh *mesh = lt_expect_LTMesh(L, 1);
    mesh->print();
    return 0;
}

static const LTEnumConstant DrawMode_enum_vals[] = {
    {"triangles",       LT_DRAWMODE_TRIANGLES},
    {"triangle_strip",  LT_DRAWMODE_TRIANGLE_STRIP},
    {"triangle_fan",    LT_DRAWMODE_TRIANGLE_FAN},
    {"points",          LT_DRAWMODE_POINTS},
    {"lines",           LT_DRAWMODE_LINES},
    {"line_strip",      LT_DRAWMODE_LINE_STRIP},
    {"line_loop",       LT_DRAWMODE_LINE_LOOP},
    {NULL, 0}
};

static LTObject *get_texture(LTObject *obj) {
    return ((LTMesh*)obj)->texture;
}

static void set_texture(LTObject *obj, LTObject *val) {
    LTMesh *mesh = (LTMesh*)obj;
    //LTTexturedNode *old_texture = mesh->texture;
    LTTexturedNode *new_texture = (LTTexturedNode*)val;
    mesh->texture = new_texture;
}

LT_REGISTER_TYPE(LTMesh, "lt.Mesh", "lt.SceneNode")
LT_REGISTER_FIELD_ENUM(LTMesh, draw_mode, LTDrawMode, DrawMode_enum_vals)
LT_REGISTER_PROPERTY_OBJ(LTMesh, texture, LTTexturedNode, get_texture, set_texture);
LT_REGISTER_METHOD(LTMesh, Clone, clone_mesh)
LT_REGISTER_METHOD(LTMesh, Stretch, stretch_mesh)
LT_REGISTER_METHOD(LTMesh, Shift, shift_mesh)
LT_REGISTER_METHOD(LTMesh, Merge, merge_mesh)
LT_REGISTER_METHOD(LTMesh, Grid, make_grid)
LT_REGISTER_METHOD(LTMesh, SetXYs, set_xys)
LT_REGISTER_METHOD(LTMesh, SetXYZs, set_xyzs)
LT_REGISTER_METHOD(LTMesh, SetRGBs, set_rgbs)
LT_REGISTER_METHOD(LTMesh, SetUVs, set_uvs)
LT_REGISTER_METHOD(LTMesh, SetIndices, set_indices)
LT_REGISTER_METHOD(LTMesh, ComputeNormals, compute_normals)
LT_REGISTER_METHOD(LTMesh, Print, print_mesh)
