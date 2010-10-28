#include <stdio.h>
#include <Cocoa/Cocoa.h>

@interface DemoView : NSView
{
}
- (void)drawRect:(NSRect)rect;
@end

@implementation DemoView
#define X(t) (sin(t)+1) * width * 0.5
#define Y(t) (cos(t)+1) * height * 0.5
- (void)drawRect:(NSRect)rect {
    double f,g;
    double const pi = 2 * acos(0.0);
    int n = 12;
    float width = [self bounds].size.width;
    float height = [self bounds].size.height;

    [[NSColor whiteColor] set];
    NSRectFill([self bounds]);

    [[NSColor blackColor] set];

    for (f=0; f<2*pi; f+=2*pi/n) {
        for (g=0; g<2*pi; g+=2*pi/n) {
            NSPoint p1 = NSMakePoint(X(f),Y(f));
            NSPoint p2 = NSMakePoint(X(g),Y(g));
            [NSBezierPath strokeLineFromPoint: p1 toPoint:p2];
        }
    }
}
- (void)windowWillClose:(NSNotification *)notification {
    [NSApp terminate:self];
}
@end

void setup() {
    NSWindow    *myWindow;
    NSView      *myView;
    NSRect      graphicsRect;

    graphicsRect = NSMakeRect(100.0, 350.0, 400.0, 400.0);

    myWindow = [[NSWindow alloc]
        initWithContentRect: graphicsRect
        styleMask: NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask
        backing: NSBackingStoreBuffered
        defer:NO
    ];
    [myWindow setTitle:@"Tiny App Window"];
    myView = [[[DemoView alloc] initWithFrame:graphicsRect] autorelease];
    [myWindow setContentView:myView];
    [myWindow setDelegate:myView];
    [myWindow makeMainWindow];
    [myWindow makeKeyAndOrderFront: nil];
}

int main(int argc, char *argv[]) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSApp = [NSApplication sharedApplication];
    setup();
    printf("start\n");
    [NSApp run];
    printf("end\n");
    [NSApp release];
    [pool release];
    return(0);
}
