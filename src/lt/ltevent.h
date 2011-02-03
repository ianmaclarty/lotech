/* Copyright (C) 2011 Ian MacLarty */
#ifndef LTEVENT_H
#define LTEVENT_H

#include "ltcommon.h"

struct LTSceneNode;

enum LTPointerEventType {
    LT_EVENT_POINTER_DOWN,
    LT_EVENT_POINTER_UP,
    LT_EVENT_POINTER_MOVE
};

struct LTPointerEvent {
    LTPointerEventType type;
    LTfloat orig_x;
    LTfloat orig_y;
    int id;

    LTPointerEvent(LTPointerEventType type, LTfloat x, LTfloat y, int id) {
        LTPointerEvent::type = type;
        LTPointerEvent::orig_x = x;
        LTPointerEvent::orig_y = y;
        LTPointerEvent::id = id;
    }
};

struct LTPointerEventHandler {
    virtual ~LTPointerEventHandler() {}

    // x and y are local coordinates w.r.t the node.
    // Returning true stops the event propogating further.
    virtual bool consume(LTfloat x, LTfloat y, LTSceneNode *node, LTPointerEvent *event) = 0;
};

//---------------------------------------------------------------

/* An action to take in response to an event. */
struct LTAction {
    virtual ~LTAction() {};

    virtual void doAction() = 0;
};

/* Queue an action to be run on the next call to ltFirePendingActions. */
void ltQueueAction(LTAction *action);
/* Run all pending actions and clear the action queue. */
void ltFirePendingActions();

#endif
