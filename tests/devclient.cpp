#include <unistd.h>

#include "lt.h"

#define DELAY ((int)((1.0 / 60.0) * 1000000.0))

int main() {
    int t = 0;
    ltClientInit();
    while (true) {
        ltClientStep();
        usleep(DELAY);
        if (t % 60 == 0) {
            printf(".");
            fflush(NULL);
        }
        t++;
    }
}
