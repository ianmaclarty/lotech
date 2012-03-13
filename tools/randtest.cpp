#include <stdio.h>
#include <time.h>
#include "ltrandom.h"

#define SAMPLE_SIZE 1000000

static void int_test(LTRandomGenerator *r, int n);
static void bool_test(LTRandomGenerator *r);

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
