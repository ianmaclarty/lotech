#include <stdlib.h>
#include <stdio.h>

#include "lt.h"

#define NUM_SQUARES 100

static LTSceneNode scene;
static LTUnitSquare bg;

struct AddRandomTween : LTAction {
    LTTweenFloatField   field;
    LTSceneNode         *node;

    AddRandomTween(LTSceneNode *n, LTTweenFloatField fld) : LTAction() {
        field = fld;
        node = n;
    }
    
    virtual void doAction() {
        LTfloat period = ltRandFloat(2.4f, 5.2f);
        LTfloat target;
        switch (field) {
            case LT_FIELD_X: target = ltRandFloat(-2.0f, 2.0f); break;
            case LT_FIELD_Y: target = ltRandFloat(-2.0f, 2.0f); break;
            case LT_FIELD_X_SCALE: target = ltRandFloat(0.5f, 1.5f); break;
            case LT_FIELD_Y_SCALE: target = ltRandFloat(0.5f, 1.5f); break;
            case LT_FIELD_ANGLE: target = ltRandFloat(0.0f, 360.0f); break;
            default: target = ltRandFloat(0.0f, 1.0f);
        }
        node->linearTween(field, target, period, this);
    }
};

static void RenderScene() {
    ltViewPort2D(-1.0f, -1.0f, 1.0f, 1.0f);
    ltRenderScene(&scene);
    ltColor(1.0f, 0.0f, 0.0f, 0.5f);
    ltRect(-0.5f, -0.5f, 0.5f, 0.5f);
}

static void Advance() {
    ltAdvanceTweens();
    ltFirePendingActions();
}

static void Setup() {
    bg.x_scale = 10.0f;
    bg.y_scale = 10.0f;
    scene.children.push_back(&bg);
    for (int i = 0; i < NUM_SQUARES; i++) {
        LTUnitSquare *square = new LTUnitSquare();
        scene.children.push_back(square);
        ltQueueAction(new AddRandomTween(square, LT_FIELD_X));
        ltQueueAction(new AddRandomTween(square, LT_FIELD_Y));
        ltQueueAction(new AddRandomTween(square, LT_FIELD_RED));
        ltQueueAction(new AddRandomTween(square, LT_FIELD_GREEN));
        ltQueueAction(new AddRandomTween(square, LT_FIELD_BLUE));
        ltQueueAction(new AddRandomTween(square, LT_FIELD_ALPHA));
        ltQueueAction(new AddRandomTween(square, LT_FIELD_X_SCALE));
        ltQueueAction(new AddRandomTween(square, LT_FIELD_Y_SCALE));
        ltQueueAction(new AddRandomTween(square, LT_FIELD_ANGLE));
    }
}

int main(int argc, char* argv[]) {
    ltHarnessInit(false, "Test", 60, Setup, NULL, RenderScene, Advance, NULL, NULL, NULL, NULL, NULL);
    return 0;
}
