/*
 * ./pngbb in.png out.png.
 * Reads in.png and generates out.png which contains only the pixels
 * in the bounding box of in.png, along with a private chunk containing the
 * bounding box information (see ltWriteImage function).
 */
#include "lt.h"

int main(int argc, const char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage error: expecting exactly 2 args.\n");
        exit(1);
    }
    LTImageBuffer *buf = ltReadImage(argv[1], "pngbb");
    if (buf != NULL) {
        ltWriteImage(argv[2], buf);
        delete buf;
        return 0;
    } else {
        return 1;
    }
}
