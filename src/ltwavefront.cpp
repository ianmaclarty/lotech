/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"
LT_INIT_IMPL(ltwavefront)

struct t_vertex {
    LTfloat x;
    LTfloat y;
    LTfloat z;
};

struct t_texture_coord {
    LTfloat u;
    LTfloat v;
};

struct t_normal {
    LTfloat nx;
    LTfloat ny;
    LTfloat nz;
};

struct t_face_component {
    long v;
    long t;
    long n;
};

typedef std::vector<t_face_component> t_face;

char *skip_line(char *str) {
    while (*str != '\n' && *str != '\0') {
        str++;
    }
    if (*str == '\0') {
        return str;
    } else {
        return str + 1;
    }
}

bool ltReadWavefrontMesh(const char *filename, LTMesh *mesh) {
    // Init obj in case we return false.
    new (mesh) LTMesh(0, false, false, false, NULL, LT_DRAWMODE_TRIANGLES, NULL, 0);

    char *str0 = ltReadTextResource(filename);
    char *str = str0;
    if (str == NULL) {
        // ltReadTextFile would have already emitted an error.
        return false;
    }
    std::vector<t_vertex> vertices;
    std::vector<t_texture_coord> texture_coords;
    std::vector<t_normal> normals;
    std::vector<t_face> faces;

    int line = 1;
    while (*str != '\0') {
        switch (*str) {
            case '#':
            case 's':
                break;
            case 'v': {
                str++;
                switch (*str) {
                    case ' ': {
                        str++;
                        // t_vertex
                        t_vertex v;
                        v.x = strtof(str, &str);
                        v.y = strtof(str, &str);
                        v.z = strtof(str, &str);
                        vertices.push_back(v);
                        break;
                    }
                    case 't': {
                        str++;
                        // texture coord
                        t_texture_coord t;
                        t.u = strtof(str, &str);
                        t.v = strtof(str, &str); // XXX assuming v component is present
                        texture_coords.push_back(t);
                        break;
                    }
                    case 'n': {
                        str++;
                        // t_normal
                        t_normal n;
                        n.nx = strtof(str, &str);
                        n.ny = strtof(str, &str);
                        n.nz = strtof(str, &str);
                        normals.push_back(n); // XXX assuming t_normal is unit.
                        break;
                    }
                    case 'p': {
                        // parameter space vertices
                        ltLog("%s:%d: Warning: parameter space vertices not supported", filename, line);
                        break;
                    }
                    default: {
                        ltLog("%s:%d: Unrecognised definition", filename, line);
                        return false;
                    }
                }
                break;
            }
            case 'f': {
                str++;
                // t_face
                t_face fa;
                while (*str != '\n') {
                    t_face_component fc;
                    fc.v = strtol(str, &str, 10);
                    if (*str == '/') {
                        str++;
                        if (*str == '/') {
                            fc.t = 0;
                            str++;
                        } else {
                            fc.t = strtol(str, &str, 10);
                        }
                        if (*str >= '0' && *str <= '9') {
                            fc.n = strtol(str, &str, 10);
                        } else {
                            fc.n = 0;
                        }
                    }
                    fa.push_back(fc);
                    while (*str == ' ') {
                        str++;
                    }
                }
                faces.push_back(fa);
                break;
            }
            default: {
                ltLog("%s:%d: Warning: unrecognised definition", filename, line);
                break;
            }
        }
        str = skip_line(str);
        line++;
    }

    free(str0);

    int num_faces = faces.size();
    if (num_faces == 0) {
        ltLog("%s: Error: no faces", filename);
        return false;
    }

    // Check all faces are triangles
    for (int i = 0; i < num_faces; i++) {
        if (faces[i].size() != 3) {
            ltLog("%s: Sorry, only triangle faces are currently supported", filename);
            return false;
        }
    }

    bool has_normals = faces[0][0].n > 0;
    bool has_texture_coords = faces[0][0].t > 0;   
    int stride = 4 * 3;
    if (has_normals) {
        stride += 4;
    }
    if (has_texture_coords) {
        stride += 4;
    }

    void *data = malloc(stride * 3 * num_faces);
    void *ptr = data;
    for (int i = 0; i < num_faces; i++) {
        for (int j = 0; j < 3; j++) {
            int v = faces[i][j].v - 1;
            if (v < 0) {
                ltLog("%s: Error: missing vertex in face %d, vertex %d", filename, i, j);
                return false;
            }
            int n = faces[i][j].n - 1;
            if (has_normals && n < 0) {
                ltLog("%s: Error: missing normal in face %d, t_vertex %d", filename, i, j);
                return false;
            }
            int t = faces[i][j].t - 1;
            if (has_texture_coords && t < 0) {
                ltLog("%s: Error: missing texture coords in face %d, vertex %d", filename, i, j);
                return false;
            }

            LTfloat *fptr = (LTfloat*)ptr;
            fptr[0] = vertices[v].x;
            fptr[1] = vertices[v].y;
            fptr[2] = vertices[v].z;
            lt_incr_ptr(&ptr, 3 * 4);
            if (n >= 0) {
                LTbyte *bptr = (LTbyte*)ptr;
                bptr[0] = (LTbyte)(normals[n].nx * 127.0f);
                bptr[1] = (LTbyte)(normals[n].ny * 127.0f);
                bptr[2] = (LTbyte)(normals[n].nz * 127.0f);
                bptr[3] = '\0';
                lt_incr_ptr(&ptr, 4);
            }
            if (t >= 0) {
                LTshort *sptr = (LTshort*)ptr;
                sptr[0] = (LTshort)(texture_coords[t].u * (LTfloat)LT_MAX_TEX_COORD);
                sptr[1] = (LTshort)(texture_coords[t].v * (LTfloat)LT_MAX_TEX_COORD);
                lt_incr_ptr(&ptr, 4);
            }
        }
    }

    mesh->~LTMesh(); // decontruct before constructing again.
    new (mesh) LTMesh(3, false, has_normals, has_texture_coords, NULL, LT_DRAWMODE_TRIANGLES, data, num_faces * 3);
    return true;
}
