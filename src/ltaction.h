struct LTSceneNode;

struct LTAction {
    // Should return true when finished.
    virtual bool doAction(LTfloat dt) = 0;
};
