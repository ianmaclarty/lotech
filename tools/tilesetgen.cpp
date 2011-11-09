#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "png.h"

#include "lt.h"

#define STRBUFSIZE 1024

static int num_cols = 4;
static int offset = 0;
static const char *name = NULL;

static void usage_error() {
    fprintf(stderr, "Usage: tilesetgen -n <name> [-c <num cols>] [-o <offset>] <png files>\n");
    exit(1);
}

static void read_options(int argc, const char **argv, int *shift) {
    int i;
    char *end_ptr;
    *shift = 0;
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            i++;
            if (i < argc) {
                const char *num_cols_str = argv[i];
                num_cols = (int)strtol(num_cols_str, &end_ptr, 10);
                if (*end_ptr != '\0' || num_cols <= 0) {
                    fprintf(stderr, "Error: Invalid value for -c option.\n"),
                    usage_error();
                }
                *shift += 2;
            } else {
                usage_error();
            }
        } else if (strcmp(argv[i], "-o") == 0) {
            i++;
            if (i < argc) {
                const char *offset_str = argv[i];
                offset = (int)strtol(offset_str, &end_ptr, 10);
                if (*end_ptr != '\0' || offset < 0) {
                    fprintf(stderr, "Error: Invalid value for -o option.\n"),
                    usage_error();
                }
                *shift += 2;
            } else {
                usage_error();
            }
        } else if (strcmp(argv[i], "-n") == 0) {
            i++;
            if (i < argc) {
                name = argv[i];
                *shift += 2;
            } else {
                usage_error();
            }
        }
    }
}

int main(int argc, const char **argv) {
    char strbuf[STRBUFSIZE];

    int shift = 0;
    if (argc <= 4) {
        usage_error();
    }
    argc--;
    argv++; // skip first arg (which is the program name).
    read_options(argc, argv, &shift);
    argc -= shift;
    argv += shift;
    if (name == NULL || strlen(name) == 0) {
        fprintf(stderr, "Error: Invalid or missing -n option.\n");
        usage_error();
    }

    // Read images.
    int num_imgs = argc;
    LTImageBuffer **imgs = new LTImageBuffer*[num_imgs];
    for (int i = 0; i < argc; i++) {
        const char* pngfile = argv[i];
        if (strlen(pngfile) < 5 || strcmp(pngfile + strlen(pngfile) - 4, ".png") != 0) {
            fprintf(stderr, "Error: Expecting argument %d to have extension .png\n", i);
            exit(1);
        }
        strncpy(strbuf, pngfile, strlen(pngfile) - 4);
        strbuf[strlen(pngfile) - 4] = '\0';
        imgs[i] = ltReadImage(argv[i], strbuf);
    }

    // Check images are all square and the same size.
    int tile_width = imgs[0]->width;
    if (tile_width != imgs[0]->height) {
        fprintf(stderr, "Error: %s is not square\n", argv[0]);
        exit(1);
    }
    for (int i = 1; i < num_imgs; i++) {
        if (imgs[i]->width != tile_width) {
            fprintf(stderr, "Error: %s has width %d, but %s has width %d\n", argv[0], tile_width, argv[i], imgs[i]->width);
            exit(1);
        }
        if (imgs[i]->height != tile_width) {
            fprintf(stderr, "Error: %s is not square\n", argv[i]);
            exit(1);
        }
    }

    // Create the tileset buffer.
    int tileset_width = tile_width * num_cols;
    int num_rows = num_imgs / num_cols + ( (num_imgs % num_cols) > 0 ? 1 : 0 );
    int tileset_height = tile_width * num_rows;
    LTImageBuffer *target = ltCreateEmptyImageBuffer("tileset", tileset_width, tileset_height);

    // Paste images into buffer.
    for (int col = 0; col < num_cols; col++) {
        for (int row = 0; row < num_rows; row++) {
            int i = col + num_cols * row;
            if (i < num_imgs) {
                int x = col * tile_width + imgs[i]->bb_left;
                int y = tileset_height - row * tile_width - tile_width + imgs[i]->bb_bottom;
                ltPasteImage(imgs[i], target, x, y, false);
            }
        }
    }
    
    // Output tileset image.
    snprintf(strbuf, STRBUFSIZE, "%s.png", name);
    ltWriteImage(strbuf, target);

    // Output a lua file that returns a map from the tile indices to the image names
    // (without the .png suffixes).
    snprintf(strbuf, STRBUFSIZE, "%s.lua", name);
    FILE* f = fopen(strbuf, "w");
    fprintf(f, "return {\n");
    for (int i = 0; i < num_imgs; i++) {
        fprintf(f, "    [%d] = \"%s\",\n", i, imgs[i]->name);
    }
    fprintf(f, "}\n");
    fclose(f);

    // Output .tsx file for use with Tiled map editor.
    snprintf(strbuf, STRBUFSIZE, "%s.tsx", name);
    f = fopen(strbuf, "w");
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<tileset name=\"%s\" tilewidth=\"%d\" tileheight=\"%d\">\n", name, tile_width, tile_width);
    fprintf(f, "  <tileoffset x=\"-%d\" y=\"%d\"/>\n", offset, offset);
    fprintf(f, "  <image source=\"%s.png\" width=\"%d\" height=\"%d\"/>\n", name, tileset_width, tileset_height);
    fprintf(f, "</tileset>\n");
    fclose(f);

    // Cleanup.
    for (int i = 0; i < num_imgs; i++) {
        delete imgs[i];
    }
    delete[] imgs;
    delete target;

    return 0;
}
