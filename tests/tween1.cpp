#include <stdio.h>
#include "lt.h"

struct S : public LTTweenable {
    float x;
    float y;
};

struct A : public LTAction {
    const char *msg;
    A(const char *msg) {
        A::msg = msg;
    };
    virtual void doAction() {
        fprintf(stderr, "%s", msg);
        fflush(NULL);
    };
};

struct B : public LTAction {
    S *s;
    B(S *s) {
        B::s = s;
    };
    virtual void doAction() {
        fprintf(stderr, "finished tween 3\n");
        fflush(NULL);
        s->addTween(new LTLinearTween(&s->x, 4.0f, 8.0f));
    };
};

int main() {
    LT_step_length = 0.5f;
    LTfloat time = 0.0f;
    bool replaced = false;
    S s1;
    A a1("finished tween 1\n");
    A a2("finished tween 2\n");
    A a4("finished tween 4\n");
    s1.x = 0.0f;
    s1.y = 0.0f;
    s1.addTween(new LTLinearTween(&s1.x, 2.0f, 5.0f, &a1));
    s1.addTween(new LTLinearTween(&s1.y, 1.0f, 10.0f, &a2));
    S *s2 = new S();
    B *a3 = new B(s2);
    s2->x = 1.0f;
    s2->y = -1.0f;
    s2->addTween(new LTLinearTween(&s2->x, -2.0f, 2.0f, a3));
    s2->addTween(new LTLinearTween(&s2->y, 2.0f, 4.0f));
    while (time < 11.0f) {
        fprintf(stderr, "time = %#.2f, s1.x = %#.2f, s1.y = %#.2f", time, s1.x, s1.y);
        if (s2 != NULL) {
            fprintf(stderr, ", s2.x = %#.2f, s2.y = %#.2f", s2->x, s2->y);
        }
        fprintf(stderr, "\n");
        fflush(NULL);
        if (time > 4.9f && s2 != NULL) {
            delete s2;
            s2 = NULL;
        }
        if (time > 6.9f && !replaced) {
            s1.addTween(new LTLinearTween(&s1.y, 10.0f, 3.0f, &a4));
            replaced = true;
        }
        ltAdvanceTweens();
        ltFirePendingActions();
        time += LT_step_length;
    }
    delete a3;
    return 0;
}
