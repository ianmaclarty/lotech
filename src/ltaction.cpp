#include "lt.h"

LT_INIT_IMPL(ltaction)

static std::list<LTAction*> action_list;
static std::list<LTAction*>::iterator next_action = action_list.end();
static std::list<LTAction*> cancelled_actions;

LTAction::LTAction(LTSceneNode *n) {
    position = action_list.end();
    node = n;
    id = NULL;
    no_dups = false;
    cancelled = false;
}

LTAction::~LTAction() {
    if (position != action_list.end()) {
        unschedule();
    }
}

void LTAction::schedule() {
    if (position != action_list.end()) {
        ltLog("LTAction::schedule: already scheduled");
        ltAbort();
    }
    action_list.push_front(this);
    position = action_list.begin();
}

void LTAction::unschedule() {
    if (position == action_list.end()) {
        ltLog("LTAction::unschedule: not scheduled");
        ltAbort();
    }
    if (position == next_action) {
        next_action = action_list.erase(position);
    } else {
        action_list.erase(position);
    }
    position = action_list.end();
} 

void LTAction::cancel() {
    if (!cancelled) {
        cancelled_actions.push_back(this);
        cancelled = true;
    }
}

void ltExecuteActions(LTfloat dt) {
    next_action = action_list.begin();
    while (next_action != action_list.end()) {
        LTAction *action = *next_action;
        next_action++;
        bool finished = action->doAction(dt);
        if (finished) {
            action->cancel();
        }
    }
    for (std::list<LTAction*>::iterator it = cancelled_actions.begin(); it != cancelled_actions.end(); it++) {
        LTAction *action = *it;
        if (action->position != action_list.end()) {
            action->unschedule();
        }
        action->node->actions->remove(action);
        delete action;
    }
    cancelled_actions.clear();
}

int ltNumScheduledActions() {
    return action_list.size();
}
