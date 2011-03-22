#include "ltios.h"
#include "ltcommon.h"
#include "ltgraphics.h"
#include "ltiosutil.h"
#include "ltlua.h"
#include "ltprotocol.h"

@interface LTViewController : UIViewController
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation;
@end

@implementation LTViewController
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    switch(ltGetDisplayOrientation()) {
        case LT_DISPLAY_ORIENTATION_PORTRAIT: 
            return (interfaceOrientation == UIInterfaceOrientationPortrait
                || interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown);
        case LT_DISPLAY_ORIENTATION_LANDSCAPE:
            return (interfaceOrientation == UIInterfaceOrientationLandscapeRight
                || interfaceOrientation == UIInterfaceOrientationLandscapeLeft);
    }
    return NO;
}
@end

static LTViewController *view_controller = nil;

// The following is required for converting UITouch objects to input_ids.
ct_assert(sizeof(UITouch*) == sizeof(int));

void ltIOSInit(UIView *view) {
    view_controller = [[LTViewController alloc] init];
    view_controller.view = view;
    [view_controller retain];
    [view removeFromSuperview];
    UIWindow *win = [[UIApplication sharedApplication] keyWindow];
    [win addSubview:view_controller.view];

    #ifdef LTDEVMODE
    ltClientInit();
    #endif

    ltSetScreenSize(ltIOSScreenWidth(), ltIOSScreenHeight());

    const char *path = ltIOSBundlePath("main", ".lua");
    ltLuaSetup(path);
    delete[] path;
}

void ltIOSTeardown() {
    ltLuaTeardown();
    [view_controller release];
    view_controller = nil;
}

void ltIOSRender() {
    ltLuaRender();
    ltLuaAdvance();

    #ifdef LTDEVMODE
    ltClientStep();
    #endif
}

void ltIOSGarbageCollect() {
    ltLuaGarbageCollect();
}

void ltIOSTouchesBegan(NSSet *touches) {
    NSEnumerator *e = [touches objectEnumerator];
    UITouch *touch;
    while (touch = [e nextObject]) {
        CGPoint pos = [touch locationInView:touch.view];
        ltLuaPointerDown((int)touch, pos.x, pos.y);
    }
}

void ltIOSTouchesMoved(NSSet *touches) {
    NSEnumerator *e = [touches objectEnumerator];
    UITouch *touch;
    while (touch = [e nextObject]) {
        CGPoint pos = [touch locationInView:touch.view];
        ltLuaPointerMove((int)touch, pos.x, pos.y);
    }
}

void ltIOSTouchesEnded(NSSet *touches) {
    NSEnumerator *e = [touches objectEnumerator];
    UITouch *touch;
    while (touch = [e nextObject]) {
        CGPoint pos = [touch locationInView:touch.view];
        ltLuaPointerUp((int)touch, pos.x, pos.y);
    }
}

void ltIOSTouchesCancelled(NSSet *touches) {
    ltIOSTouchesEnded(touches);
}

static float scaling() {
    float scale = 1.0f;
    if([[UIScreen mainScreen] respondsToSelector: NSSelectorFromString(@"scale")]) {
        scale = [[UIScreen mainScreen] scale];
    }
    return scale;
}

int ltIOSScreenWidth() {
    float scale = scaling();
    int w = view_controller.view.bounds.size.width * scale;
    return w;
}

int ltIOSScreenHeight() {
    float scale = scaling();
    return view_controller.view.bounds.size.height * scale;
}

