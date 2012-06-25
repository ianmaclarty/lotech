LT_INIT_DECL(ltaction)

struct LTAction {
    std::list<LTAction*>::iterator position; // position in schedule.
    LTSceneNode *node;
    void *action_id;
    bool no_dups;
    bool cancelled;

    LTAction(LTSceneNode *node);
    virtual ~LTAction();

    void schedule();
    void unschedule();
    void cancel();
    virtual void on_cancel() {};
    bool is_scheduled();

    // Should return true when finished.
    virtual bool doAction(LTfloat dt) = 0;
};

void ltExecuteActions(LTfloat dt);
int ltNumScheduledActions();
