/* Copyright (C) 2010 Ian MacLarty */

#include <assert.h>

#include "lttween.h"

LTTween::LTTween(LTfloat *field, LTfloat v0, LTfloat v, LTfloat time, LTfloat (*easing)(LTfloat), LTAction *action) {
    LTTween::field = field;
    LTTween::initial_value = v0;
    LTTween::final_value = v;
    LTTween::time = time;
    LTTween::easing = easing;
    LTTween::action = action;
}

void LTTween::advance(LTfloat step) {
    // NYI
}

bool LTTween::finished() {
    // NYI
    return false;
}
