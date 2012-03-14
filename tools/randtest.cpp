#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ltrandom.h"
#include "ltimage.h"

#define SAMPLE_SIZE 10000000
//#define USERANDOM 1

static void int_test(LTRandomGenerator *r, int n);
static void bool_test(LTRandomGenerator *r);
static void rand_bw_image(LTRandomGenerator *r, int w, int h, const char *name);
static void rand_color_image(LTRandomGenerator *r, int w, int h, const char *name);
static void rand_scatter_image(LTRandomGenerator *r, int w, int h, const char *name);

static int rand_next_int(int n);
static bool rand_next_bool();
static int random_next_int(int n);
static bool random_next_bool();

#if defined(USERAND)

#define RINT(r, n) rand_next_int(n)
#define RBOOL(r) rand_next_bool()

#elif defined(USERANDOM)

#define RINT(r, n) random_next_int(n)
#define RBOOL(r) random_next_bool()

#else

#define RINT(r, n) r->nextInt(n)
#define RBOOL(r) r->nextBool()

#endif

int main() {
    unsigned int seed = time(NULL);
    srand(seed);
    srandom(seed);
    LTRandomGenerator r((int)seed);
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
    rand_color_image(&r, 281, 281, "randimage1.png");
    rand_bw_image(&r, 256, 256, "randimage2.png");
    rand_color_image(&r, 512, 512, "randimage3.png");
    rand_bw_image(&r, 64, 64, "randimage4.png");
    rand_scatter_image(&r, 500, 500, "scatterimage1.png");
    rand_scatter_image(&r, 281, 281, "scatterimage2.png");
    rand_scatter_image(&r, 256, 256, "scatterimage3.png");
    rand_scatter_image(&r, 128, 128, "scatterimage4.png");
    rand_scatter_image(&r, 64, 64, "scatterimage5.png");
    return 0;
}

static void int_test(LTRandomGenerator *r, int n) {
    int *icounts = (int*)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) {
        icounts[i] = 0;
    }
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        icounts[RINT(r, n)]++;
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
        bcounts[RBOOL(r) ? 1 : 0]++;
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
            if (RBOOL(r)) {
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

static void rand_color_image(LTRandomGenerator *r, int w, int h, const char *name) {
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
            buf.bb_pixels[j * w + i] = colors[RINT(r, ncols)];
        }
    }
    ltWriteImage(name, &buf);
    printf("Generated %s\n", name);
}

static void rand_scatter_image(LTRandomGenerator *r, int w, int h, const char *name) {
    int n = w * h * 128;
    LTImageBuffer buf(name);
    buf.width = w;
    buf.height = h;
    buf.bb_left = 0;
    buf.bb_top = h - 1;
    buf.bb_right = w - 1;
    buf.bb_bottom = 0;
    buf.bb_pixels = new LTpixel[w * h];
    memset(buf.bb_pixels, 0xFF, w * h * 4);
    for (int i = 0; i < n; i++) {
        int x = RINT(r, w);
        int y = RINT(r, h);
        int val = (buf.bb_pixels[y * w + x] & 0xFF000000) >> 24;
        if (val > 0) {
            val--;
        }
        buf.bb_pixels[y * w + x] = (val << 8) | (val << 16) | (val << 24) | 0xFF;
    }
    ltWriteImage(name, &buf);
    printf("Generated %s\n", name);
}

static int rand_next_int(int n) {
    int m = n - 1;
    m |= m >> 1; 
    m |= m >> 2; 
    m |= m >> 4; 
    m |= m >> 8; 
    m |= m >> 16;
    int r;
    do {
        r = rand() & m;
    } while (r >= n);
    return r;
}

static int random_next_int(int n) {
    int m = n - 1;
    m |= m >> 1; 
    m |= m >> 2; 
    m |= m >> 4; 
    m |= m >> 8; 
    m |= m >> 16;
    int r;
    do {
        r = random() & m;
    } while (r >= n);
    return r;
}

bool rand_next_bool() {
    static const int b = 1 << 27; // Pick a bit to use.
    return rand() & b;
}

bool random_next_bool() {
    static const int b = 1 << 27; // Pick a bit to use.
    return random() & b;
}
