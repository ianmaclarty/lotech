/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(lttween)

typedef LTfloat(*LTEaseFunc)(LTfloat);

struct LTTweenAction;

struct LTTweenOnDone {
    virtual ~LTTweenOnDone() {};
    virtual void done(LTAction *action) = 0;
    virtual void on_cancel() {};
};

struct LTTweenAction : LTAction {
    LTFloatGetter getter;
    LTFloatSetter setter;
    LTfloat t;
    LTfloat initial_val;
    LTfloat target_val;
    LTfloat distance;
    LTfloat time;
    LTfloat delay;
    LTEaseFunc ease;
    LTTweenOnDone *on_done;

    LTTweenAction(LTSceneNode *node, 
        LTFloatGetter getter, LTFloatSetter setter,
        LTfloat target_val, LTfloat time, LTfloat delay, LTEaseFunc ease,
        LTTweenOnDone *on_done);
    virtual ~LTTweenAction();
    virtual void on_cancel();

    virtual bool doAction(LTfloat dt);
};

struct LTIntTweenAction : LTAction {
    LTIntGetter getter;
    LTIntSetter setter;
    LTfloat t;
    LTfloat initial_val;
    LTfloat target_val;
    LTfloat distance;
    LTfloat time;
    LTfloat delay;
    LTEaseFunc ease;
    LTTweenOnDone *on_done;

    LTIntTweenAction(LTSceneNode *node, 
        LTIntGetter getter, LTIntSetter setter,
        LTfloat target_val, LTfloat time, LTfloat delay, LTEaseFunc ease,
        LTTweenOnDone *on_done);
    virtual ~LTIntTweenAction();
    virtual void on_cancel();

    virtual bool doAction(LTfloat dt);
};

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
