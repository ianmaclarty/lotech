LT_INIT_DECL(ltaction)

struct LTAction {
    std::list<LTAction*>::iterator position; // position in schedule.
    LTSceneNode *node;

    LTAction();
    virtual ~LTAction();

    void schedule();
    void unschedule();

    // Should return true when finished.
    virtual bool doAction(LTfloat dt) = 0;
};

void ltExecuteActions(LTfloat dt);
