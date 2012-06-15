/* Copyright (C) 2010 Ian MacLarty */
LT_INIT_DECL(lttween)

typedef LTfloat(*LTEaseFunc)(LTfloat);

struct LTTween {
    LTObject *owner;
    LTFloatGetter getter;
    LTFloatSetter setter;
    LTfloat t;
    LTfloat v0;
    LTfloat v;
    LTfloat time;
    LTfloat delay;
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
    int add(LTObject *owner, LTFloatGetter getter, LTFloatSetter setter,
        LTfloat target_val, LTfloat time, LTfloat delay, LTEaseFunc ease, int slot);
};

// Returns true if finished.
bool ltAdvanceTween(LTTween *tween, LTfloat dt);

void ltInitTween(LTTween *tween, LTObject *owner, LTFloatGetter getter, LTFloatSetter setter,
    LTfloat v, LTfloat time, LTfloat delay, LTEaseFunc ease);

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
