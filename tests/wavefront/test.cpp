#include "lt.h"

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Error: expecting exactly one argument\n");
        exit(1);
    }
    LTMesh *mesh = (LTMesh*)malloc(sizeof(LTMesh));
    if (ltReadWavefrontMesh(argv[1], mesh)) {
        printf("ok\n");
    } else {
        printf("error\n");
        return 1;
    }
    mesh->print();
    free(mesh->data);
    free(mesh);
    return 0;
}
