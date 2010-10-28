/* Copyright (C) 2010 Ian MacLarty */
#include "ltrandom.h"

LTfloat ltRandFloat(LTfloat lo, LTfloat hi) {
    return lo + (hi - lo) * (rand() / (LTfloat)RAND_MAX);
}
