LT_INIT_DECL(ltaction)

struct LTSceneNode;

struct LTAction {
    // Should return true when finished.
    virtual bool doAction(LTfloat dt, LTSceneNode *node) = 0;
};
