/* Copyright (C) 2010-2011 Ian MacLarty */
#ifndef LTUTIL_H
#define LTUTIL_H

#include <stdlib.h>

void ltAbort(); // Use only for internal errors.
void ltLog(const char *fmt, ...);
bool ltFileExists(const char *file);

// Returns an array of null separated matched paths.  The last entry is two
// null characters.  The returned array should be freed by the caller with
// delete[]. The last entry in patterns should be NULL.
char* ltGlob(const char **patterns);

static inline float ltRandBetween(float lo, float hi) {
#ifdef _WIN32
    return ((float)rand() / (float)RAND_MAX) * (hi - lo) + lo;
#else
    return ((float)random() / (float)RAND_MAX) * (hi - lo) + lo;
#endif
}

static inline float ltRandMinus1_1() {
#ifdef _WIN32
    return ((float)rand() / (float)(RAND_MAX / 2)) - 1.0f;
#else
    return ((float)random() / (float)(RAND_MAX / 2)) - 1.0f;
#endif
}

static inline float ltRand0_1() {
#ifdef _WIN32
    return ((float)rand() / (float)(RAND_MAX / 2)) - 1.0f;
#else
    return (float)random() / (float)(RAND_MAX);
#endif
}

#endif