#include "lt.h"

LT_INIT_IMPL(ltaction)

static std::list<LTAction*> action_list;
static std::list<LTAction*>::iterator next_action = action_list.end();

LTAction::LTAction(LTSceneNode *node) {
    position = action_list.end();
    LTAction::node = node;
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
    action_list.push_back(this);
    position = --action_list.end();
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

void ltExecuteActions(LTfloat dt) {
    next_action = action_list.begin();
    while (next_action != action_list.end()) {
        LTAction *action = *next_action;
        next_action++;
        bool finished = action->doAction(dt);
        if (finished) {
            action->unschedule();
            action->node->actions->remove(action);
            delete action;
        }
    }
}
