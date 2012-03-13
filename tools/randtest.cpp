#include <stdio.h>
#include <time.h>
#include "ltrandom.h"
#include "ltimage.h"

#define SAMPLE_SIZE 1000000

static void int_test(LTRandomGenerator *r, int n);
static void bool_test(LTRandomGenerator *r);
static void rand_bw_image(LTRandomGenerator *r, int w, int h, const char *name);
static void rand_col_image(LTRandomGenerator *r, int w, int h, const char *name);

int main() {
    LTRandomGenerator r(time(NULL));
    int_test(&r, 1);
    int_test(&r, 2);
    int_test(&r, 3);
    int_test(&r, 4);
    int_test(&r, 5);
    int_test(&r, 6);
    int_test(&r, 7);
    int_test(&r, 8);
    int_test(&r, 9);
    int_test(&r, 10);
    int_test(&r, 20);
    int_test(&r, 30);
    bool_test(&r);
    if (ltRandomQuickCheck()) {
        printf("Quick check passed\n");
    } else {
        printf("Quick check FAILED\n");
    }
    rand_col_image(&r, 281, 281, "randimage1.png");
    rand_bw_image(&r, 300, 451, "randimage2.png");
    rand_col_image(&r, 1000, 500, "randimage3.png");
    rand_bw_image(&r, 1000, 500, "randimage4.png");
    return 0;
}

static void int_test(LTRandomGenerator *r, int n) {
    int *icounts = (int*)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) {
        icounts[i] = 0;
    }
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        icounts[r->nextInt(n)]++;
    }
    printf("N = %d:\n", n);
    for (int i = 0; i < n; i++) {
        printf("  %d: %d\n", i, icounts[i]);
    }
    free(icounts);
}

static void bool_test(LTRandomGenerator *r) {
    int bcounts[2];
    bcounts[0] = 0;
    bcounts[1] = 0;
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        bcounts[r->nextBool() ? 1 : 0]++;
    }
    printf("Bool:\n");
    printf("  false: %d\n", bcounts[0]);
    printf("  true : %d\n", bcounts[1]);
}

static void rand_bw_image(LTRandomGenerator *r, int w, int h, const char *name) {
    LTImageBuffer buf(name);
    buf.width = w;
    buf.height = h;
    buf.bb_left = 0;
    buf.bb_top = h - 1;
    buf.bb_right = w - 1;
    buf.bb_bottom = 0;
    buf.bb_pixels = new LTpixel[w * h];
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            if (r->nextBool()) {
                buf.bb_pixels[j * w + i] = 0xFFFFFFFF;
            } else {
                buf.bb_pixels[j * w + i] = 0x000000FF;
            }
        }
    }
    ltWriteImage(name, &buf);
    printf("Generated %s\n", name);
}

static LTpixel colors[] = {
    0xFF0000FF,
    0xFFFF00FF,
    0x00FF00FF,
    0x00FFFFFF,
    0x0000FFFF,
    0x000000FF,
};

static int ncols = sizeof(colors) / 4;

static void rand_col_image(LTRandomGenerator *r, int w, int h, const char *name) {
    LTImageBuffer buf(name);
    buf.width = w;
    buf.height = h;
    buf.bb_left = 0;
    buf.bb_top = h - 1;
    buf.bb_right = w - 1;
    buf.bb_bottom = 0;
    buf.bb_pixels = new LTpixel[w * h];
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            buf.bb_pixels[j * w + i] = colors[r->nextInt(ncols)];
        }
    }
    ltWriteImage(name, &buf);
    printf("Generated %s\n", name);
}
