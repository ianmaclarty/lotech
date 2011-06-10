/* Copyright (C) 2010 Ian MacLarty */
#include "lttween.h"
#include <math.h>

void ltInitTween(LTTween *tween, LTfloat *field_ptr, LTfloat delay,
    LTfloat v, LTfloat period, LTEaseFunc ease)
{
    tween->field_ptr = field_ptr;
    tween->delay = delay;
    tween->t = 0.0f;
    tween->v0 = *field_ptr;
    tween->v = v;
    tween->period = period;
    tween->ease = ease;
}

bool ltAdvanceTween(LTTween *tween, LTfloat dt) {
    LTfloat delay = tween->delay;
    if (delay > 0.0f) {
        tween->delay = delay - dt;
        return false;
    }
    LTfloat t = tween->t;
    if (t < 1.0f) {
        LTfloat v0 = tween->v0;
        LTfloat v = v0 + (tween->v - v0) * tween->ease(t);
        tween->t = t + dt / tween->period;
        *(tween->field_ptr) = v;
        return false;
    } else {
        *(tween->field_ptr) = tween->v;
        return true;
    }
}

LTfloat ltLinearEase(LTfloat t) {
    return t;
}

LTfloat ltEaseIn(LTfloat t) {
    return t * t * t;
}

LTfloat ltEaseOut(LTfloat t) {
    LTfloat t1 = t - 1.0f;
    return t1 * t1 * t1 + 1.0f;
}

LTfloat ltEaseInOut(LTfloat t) {
    t = t * 2.0f;
    if (t < 1.0f) {
        return t * t * t * 0.5f;
    }
    t = t - 2.0f;
    return t * t * t * 0.5f + 1.0f;
}

LTfloat ltBackInEase(LTfloat t) {
    static const LTfloat s = 1.70158f;
    return t * t * ((s + 1.0f) * t - s);
}

LTfloat ltBackOutEase(LTfloat t) {
    t = t - 1;
    LTfloat s = 1.70158f;
    return t * t * ((s + 1.0f) * t + s) + 1.0f;
}

LTfloat ltElasticEase(LTfloat t) {
    if (t == 0.0f or t == 1.0f) {
        return t;
    }
    LTfloat p = 0.3f;
    LTfloat s = 0.075;
    return powf(2.0f, -10.0f * t) * sinf((t - s) * (2.0f * LT_PI) / p) + 1.0f;
}

LTfloat ltBounceEase(LTfloat t) {
    static const LTfloat s = 7.5625f;
    static const LTfloat p = 2.75f;
    LTfloat l;
    if (t < 1.0f / p) {
        l = s * t * t;
    } else {
        if (t < 2.0f / p) {
            t = t - 1.5f / p;
            l = s * t * t + 0.75f;
        } else {
            if (t < 2.5f / p) {
                t = t - 2.25f / p;
                l = s * t * t + 0.9375f;
            } else {
                t = t - 2.625f / p;
                l = s * t * t + 0.984375f;
            }
        }
    }
    return l;
}

LTfloat ltAccelEase(LTfloat t) {
    return t * t;
}

LTfloat ltDeccelEase(LTfloat t) {
    LTfloat t1 = 1.0f - t;
    return 1.0f - (t1 * t1);
}

LTfloat ltZoomInEase(LTfloat t) {
    static const LTfloat s = 0.05f;
    return (1.0f / (1.0f + s - t) - 1.0f) * s;
}

LTfloat ltZoomOutEase(LTfloat t) {
    static const LTfloat s = 0.05f;
    return 1.0f + s - s / (s + t);
}

LTfloat ltRevolveEase(LTfloat t) {
    return (sinf(LT_PI * (t - 0.5f)) + 1.0f) * 0.5f;
}
