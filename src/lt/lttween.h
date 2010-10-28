/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTTWEEN_H
#define LTTWEEN_H

#include "ltcommon.h"
#include "ltevent.h"

enum LTTweenFloatField {
    LT_FIELD_X,
    LT_FIELD_Y,
    LT_FIELD_ALPHA,
    LT_FIELD_RED,
    LT_FIELD_GREEN,
    LT_FIELD_BLUE,
    LT_FIELD_X_SCALE,
    LT_FIELD_Y_SCALE,
    LT_FIELD_ANGLE
};

struct LTTweenable;

struct LTTween {
    LTTweenable     *owner;
    void            *field;
    LTAction        *onComplete;
    LTTween         *prev;
    LTTween         *next;

    LTTween(void *fld);
    virtual ~LTTween() {};

    virtual void advance() = 0;
    virtual bool isFinished() = 0;
};

struct LTTweenable {
    LTTween         *tweens;

    LTTweenable();
    virtual ~LTTweenable();

    virtual LTfloat* getFloatField(LTTweenFloatField field);

    void addTween(LTTween *tween);

    /* Convenience methods */
    void linearTween(LTTweenFloatField field, LTfloat target, LTsecs period, LTAction *onComplete = NULL);
    /*
    virtual void tweenY(LTfloat target, LTsecs period, LTAction *onComplete = NULL) {};
    virtual void tweenAlpha(LTfloat target, LTsecs period, LTAction *onComplete = NULL) {};
    virtual void tweenRed(LTfloat target, LTsecs period, LTAction *onComplete = NULL) {};
    virtual void tweenGreen(LTfloat target, LTsecs period, LTAction *onComplete = NULL) {};
    virtual void tweenBlue(LTfloat target, LTsecs period, LTAction *onComplete = NULL) {};
    virtual void tweenScale(LTfloat target, LTsecs period, LTAction *onComplete = NULL) {};
    virtual void tweenAngle(LTfloat target, LTsecs period, LTAction *onComplete = NULL) {};
    virtual void rotate(LTfloat degreesPerSec) {};
    virtual void linkToBody(b2Body *body) {};
    */
};

struct LTLinearTween : public LTTween {
    LTfloat         target;
    LTsecs          increment;

    LTLinearTween(LTfloat *fld, LTfloat target, LTsecs period, LTAction *onComplete = NULL);
    virtual ~LTLinearTween() {};
    virtual void advance();
    virtual bool isFinished();
};

void ltAdvanceTweens();

void ltCheckTweens();

#endif
