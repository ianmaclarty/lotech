/* Copyright (C) 2010 Ian MacLarty */
#include "lt.h"

LTTweenSet::LTTweenSet() : LTObject(LT_TYPE_TWEENSET) {
    LTTweenSet::capacity = 4;
    LTTweenSet::occupants = 0;
    LTTweenSet::tweens = new LTTween[capacity];
}

LTTweenSet::~LTTweenSet() {
    delete[] tweens;
}

int LTTweenSet::add(LTObject *owner, LTfloat *field_ptr, LTfloat target_val, LTfloat time, LTfloat delay, LTEaseFunc ease, int slot) {
    if (slot < 0) {
        if (occupants == capacity) {
            int new_capacity = capacity * 2;
            LTTween *new_tweens = new LTTween[new_capacity];
            memcpy(new_tweens, tweens, sizeof(LTTween) * capacity);
            delete[] tweens;
            tweens = new_tweens;
            capacity = new_capacity;
        }
        slot = occupants;
        occupants++;
    }
    LTTween *tween = &tweens[slot];
    ltInitTween(tween, owner, field_ptr, target_val, time, delay, ease);
    return slot;
}

void ltInitTween(LTTween *tween, LTObject *owner, LTfloat *field_ptr,
    LTfloat v, LTfloat time, LTfloat delay, LTEaseFunc ease)
{
    tween->owner = owner;
    tween->field_ptr = field_ptr;
    tween->t = 0.0f;
    tween->v0 = *field_ptr;
    tween->v = v;
    tween->time = time;
    tween->delay = delay;
    tween->ease = ease;
}

bool ltAdvanceTween(LTTween *tween, LTfloat dt) {
    if (tween->delay > 0) {
        tween->delay -= dt;
        return false;
    }
    LTfloat t = tween->t;
    if (t < 1.0f) {
        LTfloat v0 = tween->v0;
        LTfloat v = v0 + (tween->v - v0) * tween->ease(t);
        tween->t = t + dt / tween->time;
        *(tween->field_ptr) = v;
        return false;
    } else {
        *(tween->field_ptr) = tween->v;
        return true;
    }
}

LTfloat ltEase_linear(LTfloat t) {
    return t;
}

LTfloat ltEase_in(LTfloat t) {
    return t * t * t;
}

LTfloat ltEase_out(LTfloat t) {
    LTfloat t1 = t - 1.0f;
    return t1 * t1 * t1 + 1.0f;
}

LTfloat ltEase_inout(LTfloat t) {
    t = t * 2.0f;
    if (t < 1.0f) {
        return t * t * t * 0.5f;
    }
    t = t - 2.0f;
    return t * t * t * 0.5f + 1.0f;
}

LTfloat ltEase_backin(LTfloat t) {
    static const LTfloat s = 1.70158f;
    return t * t * ((s + 1.0f) * t - s);
}

LTfloat ltEase_backout(LTfloat t) {
    t = t - 1;
    LTfloat s = 1.70158f;
    return t * t * ((s + 1.0f) * t + s) + 1.0f;
}

LTfloat ltEase_elastic(LTfloat t) {
    if (t == 0.0f or t == 1.0f) {
        return t;
    }
    LTfloat p = 0.3f;
    LTfloat s = 0.075;
    return powf(2.0f, -10.0f * t) * sinf((t - s) * (2.0f * LT_PI) / p) + 1.0f;
}

LTfloat ltEase_bounce(LTfloat t) {
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

LTfloat ltEase_accel(LTfloat t) {
    return t * t;
}

LTfloat ltEase_decel(LTfloat t) {
    LTfloat t1 = 1.0f - t;
    return 1.0f - (t1 * t1);
}

LTfloat ltEase_zoomin(LTfloat t) {
    static const LTfloat s = 0.05f;
    return (1.0f / (1.0f + s - t) - 1.0f) * s;
}

LTfloat ltEase_zoomout(LTfloat t) {
    static const LTfloat s = 0.05f;
    return 1.0f + s - s / (s + t);
}

LTfloat ltEase_revolve(LTfloat t) {
    return (sinf(LT_PI * (t - 0.5f)) + 1.0f) * 0.5f;
}
