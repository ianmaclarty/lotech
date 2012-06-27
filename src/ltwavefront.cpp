#include "lt.h"
LT_INIT_IMPL(ltwavefront)

struct vertex {
    LTfloat x;
    LTfloat y;
    LTfloat z;
};

struct texture_coord {
    LTfloat u;
    LTfloat v;
};

struct normal {
    LTfloat nx;
    LTfloat ny;
    LTfloat nz;
};

struct face_component {
    long v;
    long t;
    long n;
};

typedef std::vector<face_component> face;

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

#define incr_ptr(ptr, n) {ptr = ((char*)ptr) + n;}

bool ltReadWavefrontMesh(const char *filename, LTMesh *mesh) {
    char *str = ltReadTextFile(filename);
    if (str == NULL) {
        // ltReadTextFile would have already emitted an error.
        return false;
    }
    std::vector<vertex> vertices;
    std::vector<texture_coord> texture_coords;
    std::vector<normal> normals;
    std::vector<face> faces;

    int line = 1;
    while (*str != '\0') {
        switch (*str) {
            case '#': {
                break;
            }
            case 'v': {
                str++;
                switch (*str) {
                    case ' ': {
                        str++;
                        // vertex
                        vertex v;
                        v.x = strtof(str, &str);
                        v.y = strtof(str, &str);
                        v.z = strtof(str, &str);
                        vertices.push_back(v);
                        break;
                    }
                    case 't': {
                        str++;
                        // texture coord
                        texture_coord t;
                        t.u = strtof(str, &str);
                        t.v = strtof(str, &str); // XXX assuming v component is present
                        texture_coords.push_back(t);
                        break;
                    }
                    case 'n': {
                        str++;
                        // normal
                        normal n;
                        n.nx = strtof(str, &str);
                        n.ny = strtof(str, &str);
                        n.nz = strtof(str, &str);
                        normals.push_back(n); // XXX assuming normal is unit.
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
                // face
                face fa;
                while (*str != '\n') {
                    face_component fc;
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
            }
            default: {
                ltLog("%s:%d: Error: unrecognised definition", filename, line);
                return false;
            }
        }
        str = skip_line(str);
        line++;
    }

    int num_faces = faces.size();
    if (num_faces == 0) {
        ltLog("%s: Error: no faces");
        return false;
    }

    // Check all faces are triangles
    for (int i = 0; i < num_faces; i++) {
        if (faces[i].size() != 3) {
            ltLog("%s: Sorry, only triangle faces are currently supported");
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
    void *ptr;
    for (int i = 0; i < num_faces; i++) {
        for (int j = 0; j < 3; j++) {
            int v = faces[i][j].v - 1;
            if (v < 0) {
                ltLog("%s: Error: missing vertex in face %d, vertex %d", filename, i, j);
                return false;
            }
            int n = faces[i][j].n - 1;
            if (has_normals && n < 0) {
                ltLog("%s: Error: missing normal in face %d, vertex", filename, i, j);
                return false;
            }
            int t = faces[i][j].t - 1;
            if (has_texture_coords && t < 0) {
                ltLog("%s: Error: missing texture coords in face %d, vertex", filename, i, j);
                return false;
            }

            LTfloat *fptr = (LTfloat*)ptr;
            fptr[0] = vertices[v].x;
            fptr[1] = vertices[v].y;
            fptr[2] = vertices[v].z;
            incr_ptr(ptr, 3 * 4);
            if (n > 0) {
                LTbyte *bptr = (LTbyte*)ptr;
                bptr[0] = (LTbyte)(normals[n].nx * 127.0f);
                bptr[1] = (LTbyte)(normals[n].ny * 127.0f);
                bptr[2] = (LTbyte)(normals[n].nz * 127.0f);
                bptr[3] = '\0';
                incr_ptr(ptr, 4);
            }
            if (t > 0) {
                LTshort *sptr = (LTshort*)ptr;
                sptr[0] = (LTshort)(texture_coords[t].u * (LTfloat)LT_MAX_TEX_COORD);
                sptr[1] = (LTshort)(texture_coords[t].v * (LTfloat)LT_MAX_TEX_COORD);
                incr_ptr(ptr, 4);
            }
        }
    }

    new (mesh) LTMesh(3, false, has_normals, has_texture_coords, NULL, LT_DRAWMODE_TRIANGLES, data, num_faces * 3);
    return true;
}
