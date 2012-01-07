/* Copyright (C) 2010 Ian MacLarty */
#include "ltevent.h"

struct LTActionList {
    LTAction        *action;
    LTActionList    *next;
};

static LTActionList *g_pending_actions = NULL;

void ltQueueAction(LTAction *action) {
    LTActionList *head = new LTActionList();
    head->action = action;
    head->next = g_pending_actions;
    g_pending_actions = head;
}

void ltFirePendingActions() {
    LTActionList *ptr = g_pending_actions;
    while (ptr != NULL) {
        LTActionList *nxt = ptr->next;
        ptr->action->doAction();
        delete ptr;
        ptr = nxt;
    }
    g_pending_actions = NULL;
}
