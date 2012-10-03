#include "lt.h"

LT_INIT_IMPL(ltaction)

static std::list<LTAction*> action_list;
static std::list<LTAction*>::iterator next_action = action_list.end();
static std::list<LTAction*> cancelled_actions;

LTAction::LTAction(LTSceneNode *n) {
    position = action_list.end();
    node = n;
    action_id = NULL;
    no_dups = false;
    cancelled = false;
    scheduled = false;
}

LTAction::~LTAction() {
    assert(!scheduled);
}

void LTAction::schedule() {
    if (scheduled) {
        ltLog("LTAction::schedule: already scheduled");
        ltAbort();
    }
    action_list.push_front(this);
    position = action_list.begin();
    scheduled = true;
}

void LTAction::unschedule() {
    if (!scheduled) {
        ltLog("LTAction::unschedule: not scheduled");
        ltAbort();
    }
    if (position == next_action) {
        next_action = action_list.erase(position);
    } else {
        action_list.erase(position);
    }
    scheduled = false;
} 

void LTAction::cancel() {
    if (!cancelled) {
        cancelled_actions.push_back(this);
        cancelled = true;
        on_cancel();
    }
}

void ltExecuteActions(LTfloat dt) {
    next_action = action_list.begin();
    while (next_action != action_list.end()) {
        LTAction *action = *next_action;
        next_action++;
        assert(action->cancelled || action->node->active);
        if (!action->cancelled && action->node->action_speed != 0.0f) {
            bool finished = action->doAction(dt * action->node->action_speed);
            if (finished) {
                action->cancel();
            }
        }
    }
    for (std::list<LTAction*>::iterator it = cancelled_actions.begin(); it != cancelled_actions.end(); it++) {
        LTAction *action = *it;
        assert(action->cancelled);
        if (action->scheduled) {
            action->unschedule();
        }
        // node == NULL implies the node has been deleted (see ltscene.cpp)
        if (action->node != NULL) {
            action->node->actions->remove(action);
        }
        delete action;
    }
    cancelled_actions.clear();
}

int ltNumScheduledActions() {
    return action_list.size();
}
