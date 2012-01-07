/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTTWEEN_H
#define LTTWEEN_H

#include "ltcommon.h"
#include "ltevent.h"

typedef LTfloat(*LTEaseFunc)(LTfloat);

// Use for generated hash table mapping strings to easing funcs.
struct LTEaseFuncInfo {
    const char *name;
    LTEaseFunc func;
};

struct LTTween {
    LTObject *owner;
    LTfloat *field_ptr;
    LTfloat t;
    LTfloat v0;
    LTfloat v;
    LTfloat time;
    LTEaseFunc ease;
};

struct LTTweenSet : LTObject {
    int capacity;
    int occupants;
    LTTween *tweens;

    LTTweenSet();
    virtual ~LTTweenSet();

    // If slot == -1, a new tween is added, otherwise the tween in the given slot is replaced.
    // Returns the slot used for the tween.
    int add(LTObject *owner, LTfloat *field_ptr, LTfloat target_val, LTfloat time, LTEaseFunc ease, int slot);
};

// Returns true if finished.
bool ltAdvanceTween(LTTween *tween, LTfloat dt);

void ltInitTween(LTTween *tween, LTObject *owner, LTfloat *field_ptr, LTfloat v, LTfloat time,
    LTEaseFunc ease);

LTfloat ltEase_linear   (LTfloat t);
LTfloat ltEase_in       (LTfloat t);
LTfloat ltEase_out      (LTfloat t);
LTfloat ltEase_inout    (LTfloat t);
LTfloat ltEase_backin   (LTfloat t);
LTfloat ltEase_backout  (LTfloat t);
LTfloat ltEase_elastic  (LTfloat t);
LTfloat ltEase_bounce   (LTfloat t);
LTfloat ltEase_accel    (LTfloat t);
LTfloat ltEase_decel    (LTfloat t);
LTfloat ltEase_zoomin   (LTfloat t);
LTfloat ltEase_zoomout  (LTfloat t);
LTfloat ltEase_revolve  (LTfloat t);

#endif
