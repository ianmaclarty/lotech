#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "png.h"

#include "lt.h"

static int texture_size = 1024;

static void usage_error() {
    fprintf(stderr, "Usage: atlasgen [-s <texsize>] <png files>\n");
    exit(1);
}

static void read_options(int argc, const char **argv, int *shift) {
    int i;
    char *end_ptr;
    *shift = 0;
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            i++;
            if (i < argc) {
                const char *size_str = argv[i];
                texture_size = (int)strtol(size_str, &end_ptr, 10);
                if (*end_ptr != '\0' || texture_size <= 0) {
                    fprintf(stderr, "Error: Invalid texture size.\n"),
                    usage_error();
                }
                *shift += 2;
            } else {
                usage_error();
            }
        }
    }
}

/*
static void print_image(LTImageBuffer *img) {
    printf("w = %d, h = %d, l = %d, t = %d, r = %d, b = %d\n",
        img->width, img->height,
        img->bb_left, img->bb_top, img->bb_right, img->bb_bottom);
    for (int row = img->bb_top; row >= img->bb_bottom; row--) {
        for (int col = img->bb_left; col <= img->bb_right; col++) {
            int i = col - img->bb_left + (row - img->bb_bottom) * img->bb_width();
            LTpixel pxl = img->bb_pixels[i];
            if (LT_ALPHA(pxl) > 0) {
                printf("x");
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
}
*/

static void bench(const char *msg) {
    static double t = 0.0;
    t = ((double)clock() / (double)CLOCKS_PER_SEC) - t;
    fprintf(stderr, "%s  %g\n", msg, t);
}

int main(int argc, const char **argv) {
    int i = 0;
    int shift = 0;
    if (argc <= 1) {
        usage_error();
    }
    argc--;
    argv++;
    read_options(argc, argv, &shift);
    argc -= shift;
    argv += shift;

    LTImagePacker *packer = new LTImagePacker(0, 0, 1024, 1024);

    for (i = 0; i < argc; i++) {
        LTImageBuffer *img = ltReadImage(argv[i]);
        if (!ltPackImage(packer, img)) {
            fprintf(stderr, "Error: Not enough space to pack %s.\n", img->file);
            exit(1);
        }
    }
    bench("load and pack ");
    LTImageBuffer *atlas = ltCreateAtlasImage("out.png", packer);
    bench("create atlas  ");
    ltWriteImage("out.png", atlas);
    bench("write         ");
    packer->deleteOccupants();
    delete atlas;
    delete packer;
    return 0;
}
