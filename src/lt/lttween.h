/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTTWEEN_H
#define LTTWEEN_H

#include "ltcommon.h"
#include "ltevent.h"

typedef LTfloat(*LTEaseFunc)(LTfloat);

struct LTTween {
    LTfloat *field_ptr;
    LTfloat delay;
    LTfloat t;
    LTfloat v0;
    LTfloat v;
    LTfloat period;
    LTEaseFunc ease;
};

// Returns true if finished.
bool ltAdvanceTween(LTTween *tween, LTfloat dt);

void ltInitTween(LTTween *tween, LTfloat *field_ptr, LTfloat delay,
    LTfloat v, LTfloat period, LTEaseFunc ease);

LTfloat ltLinearEase(LTfloat t);
LTfloat ltEaseIn(LTfloat t);
LTfloat ltEaseOut(LTfloat t);
LTfloat ltEaseInOut(LTfloat t);
LTfloat ltBackInEase(LTfloat t);
LTfloat ltBackOutEase(LTfloat t);
LTfloat ltElasticEase(LTfloat t);
LTfloat ltBounceEase(LTfloat t);

#endif
