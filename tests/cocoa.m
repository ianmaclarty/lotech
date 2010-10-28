#include <stdio.h>

#include "Cocoa/Cocoa.h"

@interface LTApplication : NSApplication
- (void)sendEvent:(NSEvent *)anEvent;
- (void)run;
@end

@implementation LTApplication
- (void)sendEvent:(NSEvent *)anEvent
{
    printf("event!\n");
    [super sendEvent:anEvent];
}
- (void)run {
    printf("run!\n");
    [super run];
}
@end

@interface LTOpenGLView: NSOpenGLView
@end

@implementation LTOpenGLView
@end

@interface LTAppDelegate : NSObject <NSApplicationDelegate>
- (void)applicationDidFinishLaunching: (NSNotification *)aNotification;
@end

@implementation LTAppDelegate
- (void)applicationDidFinishLaunching: (NSNotification *)aNotification
{
    printf("applicationDidFinishLaunching\n");
    NSRect rect = NSMakeRect(0, 0, 500, 500);
    NSScreen *screen = [NSScreen mainScreen];
    NSWindow *window = [NSWindow alloc];
    [window initWithContentRect: rect
        styleMask: NSBorderlessWindowMask
        backing: NSBackingStoreBuffered
        defer: NO
        screen: screen];

    LTOpenGLView* view = [[LTOpenGLView alloc] initWithFrame: rect];
    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, 32,
        0
    };
    NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attrs];
    NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat: format shareContext: nil];
    if (context == nil) {
        printf("Error: couldn't make context\n");
    }
    [view setOpenGLContext: context];
    [view setPixelFormat: format];
    [window setContentView: view];
    [window setReleasedWhenClosed: YES];
    [window setAcceptsMouseMovedEvents: YES];
    [window setTitle: @"LT"];
    [window makeKeyAndOrderFront:self];
    //[view enterFullScreenMode: nil withOptions: nil];
    [format release];
    [view release];
    return;
}
@end

int main(int argc, char *argv[]) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSApplication *app = [LTApplication sharedApplication];
    LTAppDelegate *delegate = [[LTAppDelegate alloc] init];

    [app setDelegate: delegate];
    [app run];
    [pool release];
    return 0;
}
