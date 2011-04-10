/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTTWEEN_H
#define LTTWEEN_H

#include "ltcommon.h"
#include "ltevent.h"

struct LTTween {
    LTfloat *field;
    LTfloat initial_value;
    LTfloat final_value;
    LTfloat time;
    LTfloat (*easing)(LTfloat);
    LTAction *action;

    LTTween(LTfloat *field, LTfloat v0, LTfloat v, LTfloat time, LTfloat (*easing)(LTfloat), LTAction *action);
    void advance(LTfloat step);
    bool finished();
};

#endif
