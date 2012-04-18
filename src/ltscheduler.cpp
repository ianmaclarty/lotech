#include "lt.h"

static std::set<LTSceneNode*> schedule;
static std::set<LTSceneNode*> to_add;
static std::set<LTSceneNode*> to_remove;
static bool executing = false;

void ltSchedulerAdd(LTSceneNode *node) {
    if (executing) {
        to_add.insert(node);
    } else {
        schedule.insert(node);
    }
}

void ltSchedulerRemove(LTSceneNode *node) {
    if (executing) {
        to_remove.insert(node);
    } else {
        schedule.erase(node);
    }
}

void ltSchedulerExecute(LTfloat dt) {
    executing = true;
    std::set<LTSceneNode*>::iterator it;
    for (it = schedule.begin(); it != schedule.end(); it++) {
        // XXX
    }
    executing = false;
}
