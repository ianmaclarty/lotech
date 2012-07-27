#ifdef LTOSX
#include "lt.h"

static LTKey get_lt_key(NSEvent *event);

static NSLock *mutex = [[NSLock alloc] init];

static bool context_changed = false;

struct event_and_pos {
    NSEvent *event;
    NSPoint pos;
};

#define MAX_EVENTS 1024
static event_and_pos event_queue[MAX_EVENTS];
static int event_queue_top = -1;

static void lock() {
    [mutex lock];
}

static void unlock() {
    [mutex unlock];
}

static int w, h;

void ltOSXInit() {
    lock();
    #ifdef LTDEVMODE
    ltClientInit();
    #endif

    ltLuaSetup();
    unlock();
}

void ltOSXTeardown() {
    lock();
    ltLuaTeardown();
    unlock();
}

void ltOSXResize(int width, int height) {
    lock();
    ltLuaResizeWindow(width, height);
    w = width;
    h = height;
    unlock();
}

void ltOSXRender() {
    static const LTdouble f = 1.0/60.0;
    static LTdouble t0 = -1.0;
    static LTdouble t_accum = 0.0;
    lock();

    if (context_changed) {
        //ltLuaPostContextChange();
        context_changed = false;
    }

    // Handle events
    while (event_queue_top >= 0) {
        event_and_pos top = event_queue[event_queue_top];
        switch ([(top.event) type]) {
            case NSLeftMouseDown:
                ltLuaMouseDown(1, top.pos.x, h - top.pos.y);
                break;
            case NSLeftMouseUp:
                ltLuaMouseUp(1, top.pos.x, h - top.pos.y);
                break;
            case NSMouseMoved:
                ltLuaMouseMove(top.pos.x, h - top.pos.y);
                break;
            case NSKeyDown:
                ltLuaKeyDown(get_lt_key(top.event));
                break;
            case NSKeyUp:
                ltLuaKeyUp(get_lt_key(top.event));
                break;
        }
        [(top.event) release];
        event_queue_top--;
    }

    // Advance
    if (t0 < 0.0) {
        t0 = [NSDate timeIntervalSinceReferenceDate];
    } else {
        LTdouble t = [NSDate timeIntervalSinceReferenceDate];
        t_accum += t - t0;
        while (t_accum >= f) {
            ltLuaAdvance(f);
            t_accum -= f;
        }
        t0 = t;
    }

    // Render
    ltLuaRender();

    #ifdef LTDEVMODE
    ltClientStep();
    #endif
    unlock();
}

static NSPoint get_pos(NSEvent *event, NSView *view) {
    if (view == nil) {
        return [event locationInWindow];
    } else {
        return [view convertPoint:[event locationInWindow] fromView:nil];
    }
}

static void push_event(NSEvent *event, NSView *view) {
    if ((event_queue_top + 1) < MAX_EVENTS) {
        event_queue_top++;
        event_queue[event_queue_top].event = event;
        [event retain];
        event_queue[event_queue_top].pos = get_pos(event, view);
    }
}

void ltOSXMouseDown(NSEvent *event, NSView *view) {
    lock();
    push_event(event, view);
    unlock();
}

void ltOSXMouseUp(NSEvent *event, NSView *view) {
    lock();
    push_event(event, view);
    unlock();
}

void ltOSXMouseMoved(NSEvent *event, NSView *view) {
    lock();
    push_event(event, view);
    unlock();
}

void ltOSXSaveState() {
    lock();
    ltSaveState();
    ltOSXSyncStore();
    unlock();
}

void ltOSXKeyUp(NSEvent *event) {
    lock();
    push_event(event, nil);
    unlock();
}

void ltOSXKeyDown(NSEvent *event) {
    if (![event isARepeat]) {
        lock();
        push_event(event, nil);
        unlock();
    }
}

void ltOSXPreContextChange() {
    //ltLuaPreContextChange();
    context_changed = true;
}

void ltOSXPostContextChange() {
}

static LTKey get_lt_key(NSEvent *event) {
    unichar key = [[event charactersIgnoringModifiers] characterAtIndex:0];
    switch (key) {
        case 'a':
        case 'A':
            return LT_KEY_A;
        case 'b':
        case 'B':
            return LT_KEY_B;
        case 'c':
        case 'C':
            return LT_KEY_C;
        case 'd':
        case 'D':
            return LT_KEY_D;
        case 'e':
        case 'E':
            return LT_KEY_E;
        case 'f':
        case 'F':
            return LT_KEY_F;
        case 'g':
        case 'G':
            return LT_KEY_G;
        case 'h':
        case 'H':
            return LT_KEY_H;
        case 'i':
        case 'I':
            return LT_KEY_I;
        case 'j':
        case 'J':
            return LT_KEY_J;
        case 'k':
        case 'K':
            return LT_KEY_K;
        case 'l':
        case 'L':
            return LT_KEY_L;
        case 'm':
        case 'M':
            return LT_KEY_M;
        case 'n':
        case 'N':
            return LT_KEY_N;
        case 'o':
        case 'O':
            return LT_KEY_O;
        case 'p':
        case 'P':
            return LT_KEY_P;
        case 'q':
        case 'Q':
            return LT_KEY_Q;
        case 'r':
        case 'R':
            return LT_KEY_R;
        case 's':
        case 'S':
            return LT_KEY_S;
        case 't':
        case 'T':
            return LT_KEY_T;
        case 'u':
        case 'U':
            return LT_KEY_U;
        case 'v':
        case 'V':
            return LT_KEY_V;
        case 'w':
        case 'W':
            return LT_KEY_W;
        case 'x':
        case 'X':
            return LT_KEY_X;
        case 'y':
        case 'Y':
            return LT_KEY_Y;
        case 'z':
        case 'Z':
            return LT_KEY_Z;
        case '0':
            return LT_KEY_0;
        case '1':
            return LT_KEY_1;
        case '2':
            return LT_KEY_2;
        case '3':
            return LT_KEY_3;
        case '4':
            return LT_KEY_4;
        case '5':
            return LT_KEY_5;
        case '6':
            return LT_KEY_6;
        case '7':
            return LT_KEY_7;
        case '8':
            return LT_KEY_8;
        case '9':
            return LT_KEY_9;
        case ' ':
            return LT_KEY_SPACE;
    }
    unsigned short code = [event keyCode];
    // See http://forums.macrumors.com/showthread.php?t=780577 for codes.
    switch (code) {
        case 0x24:
            return LT_KEY_ENTER;
        case 0x33:
            return LT_KEY_DEL;
        case 0x7E:
            return LT_KEY_UP;
        case 0x7D:
            return LT_KEY_DOWN;
        case 0x7B:
            return LT_KEY_LEFT;
        case 0x7C:
            return LT_KEY_RIGHT;
        case 0x35:
            return LT_KEY_ESC;
        default:
            #ifdef LTDEVMODE
            fprintf(stderr, "Unknown key pressed: %hu\n", code);
            #endif
            return LT_KEY_UNKNOWN;
    }
}

#endif // LTOSX
