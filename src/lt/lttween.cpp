/* Copyright (C) 2010 Ian MacLarty */

#include <assert.h>

#include "lttime.h"
#include "lttween.h"

static void checkTween(LTTween *ptr);

//------------------------------------------------------------

static LTTween *g_tweens = NULL;

//------------------------------------------------------------

LTTween::LTTween(void *fld) {
    owner = NULL;
    field = fld;
    onComplete = NULL;
    prev = NULL;
    next = NULL;
}

//------------------------------------------------------------

LTTweenable::LTTweenable() {
    tweens = NULL;
}

LTTweenable::~LTTweenable() {
    // Delete tweens for this tweenable.
    if (tweens != NULL) {
        LTTween *ptr = tweens;
        LTTween *prev = ptr->prev;
        LTTween *next = ptr;
        while (ptr != NULL && ptr->owner == this) {
            next = ptr->next;
            delete ptr;
            ptr = next;
        }
        if (prev == NULL) {
            g_tweens = next;
        } else {
            prev->next = next;
        }
        if (next != NULL) {
            next->prev = prev;
        }
    }
}

void LTTweenable::addTween(LTTween *tween) {
    tween->owner = this;
    if (tweens == NULL) {
        if (g_tweens != NULL) {
            tween->next = g_tweens;
            g_tweens->prev = tween;
        }
        g_tweens = tween;
        tweens = tween;
    } else {
        LTTween *ptr = tweens;
        // Check if any existing tweens already own
        // the field, replacing them if they do.
        while (ptr != NULL && ptr->owner == this) {
            if (ptr->field == tween->field) {
                if (ptr->prev != NULL) {
                    ptr->prev->next = tween;
                    tween->prev = ptr->prev;
                }
                if (ptr->next != NULL) {
                    ptr->next->prev = tween;
                    tween->next = ptr->next;
                }
                if (ptr == tweens) {
                    tweens = tween;
                }
                if (ptr == g_tweens) {
                    g_tweens = tween;
                }
                delete ptr;
                return;
            }
            ptr = ptr->next;
        }
        // No existing tweens own the field, so insert the new tween.
        tween->next = tweens;
        tween->prev = tweens->prev;
        if (tweens->prev != NULL) {
            tweens->prev->next = tween;
        }
        tweens->prev = tween;
        if (g_tweens == tweens) {
            g_tweens = tween;
        }
        tweens = tween;
    }
}

LTfloat* LTTweenable::getFloatField(LTTweenFloatField field) {
    return NULL;
}

void LTTweenable::linearTween(LTTweenFloatField field, LTfloat target, LTsecs period, LTAction *onComplete) {
    LTfloat *field_ptr = getFloatField(field);
    if (field_ptr != NULL) {
        LTLinearTween *tween = new LTLinearTween(field_ptr, target, period, onComplete);
        addTween(tween);
    }
};

//------------------------------------------------------------

LTLinearTween::LTLinearTween(LTfloat *fld, LTfloat target, LTsecs period, LTAction *onComplete) : LTTween(fld) {
    LTLinearTween::target = target;
    if (period != 0.0f) {
        increment = (LT_step_length / period) * (target - *fld);
    } else {
        increment = (target - *fld);
    }
    LTLinearTween::onComplete = onComplete;
}

void LTLinearTween::advance() {
    LTfloat *fld = (LTfloat*)field;
    *fld += increment;
}

bool LTLinearTween::isFinished() {
    LTfloat *fld = (LTfloat*)field;
    if (increment > 0.0f) {
        return *fld >= target;
    } else {
        return *fld <= target;
    }
}

//------------------------------------------------------------

void ltAdvanceTweens() {
    LTTween *ptr = g_tweens;
    LTTween *nxt;
    while (ptr != NULL) {
        ptr->advance();
        nxt = ptr->next;
        // Delete finished tweens, posting onComplete events if necessary.
        if (ptr->isFinished()) {
            if (ptr->onComplete != NULL) {
                ltQueueAction(ptr->onComplete);
            }
            if (ptr->prev != NULL) {
                ptr->prev->next = ptr->next;
            }
            if (ptr->next != NULL) {
                ptr->next->prev = ptr->prev;
            }
            if (ptr->owner->tweens == ptr) {
                if (ptr->next != NULL && ptr->next->owner == ptr->owner) {
                    ptr->owner->tweens = ptr->next;
                } else {
                    ptr->owner->tweens = NULL;
                }
            }
            if (ptr == g_tweens) {
                g_tweens = ptr->next;
                if (g_tweens != NULL) {
                    g_tweens->prev = NULL;
                }
            }
            delete ptr;
        }
        ptr = nxt;
    }
}

//------------------------------------------------------------

void ltCheckTweens() {
    int count = 0;
    LTTween *ptr = g_tweens;
    while (ptr != NULL) {
        fprintf(stderr, "%p -> ", ptr);
        count ++;
        checkTween(ptr);
        ptr = ptr->next;
    }
    fprintf(stderr, "NULL\nNUM TWEENS=%d\n", count);
}

static void checkTween(LTTween *ptr) {
    assert(ptr->next != ptr);
    if (ptr->next != NULL) {
        assert(ptr->next->prev == ptr);
    }
    if (ptr != g_tweens) {
        assert(ptr->prev != NULL);
        assert(ptr->prev->next == ptr);
    } else {
        assert(ptr->prev == NULL);
    }
}
