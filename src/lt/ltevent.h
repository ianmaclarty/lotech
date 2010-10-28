/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTEVENT_H
#define LTEVENT_H

#include "ltcommon.h"

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
