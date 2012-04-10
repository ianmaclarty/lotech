/* Copyright (C) 2011 Ian MacLarty */
struct LTSceneNode;

enum LTPointerEventType {
    LT_EVENT_POINTER_DOWN,
    LT_EVENT_POINTER_UP,
    LT_EVENT_POINTER_MOVE
};

struct LTPointerEvent {
    LTPointerEventType type;
    LTfloat orig_x;
    LTfloat orig_y;
    int input_id;

    LTPointerEvent(LTPointerEventType type, LTfloat x, LTfloat y, int input_id) {
        LTPointerEvent::type = type;
        LTPointerEvent::orig_x = x;
        LTPointerEvent::orig_y = y;
        LTPointerEvent::input_id = input_id;
    }
};

struct LTPointerEventHandler {
    virtual ~LTPointerEventHandler() {}

    // x and y are local coordinates w.r.t the node.
    // Returning true stops the event propogating further.
    virtual bool consume(LTfloat x, LTfloat y, LTSceneNode *node, LTPointerEvent *event) = 0;
};
