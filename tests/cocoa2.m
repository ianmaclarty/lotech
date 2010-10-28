#include <stdio.h>
#include <Cocoa/Cocoa.h>
#include <OpenGL/GL.h>

static GLfloat red = 0.0f;
static GLfloat green = 1.0f;

static CGLContextObj fullScreenContextObj;

static void render() {
    static GLfloat theta = 0.0f;
    GLfloat os = 5.0f * sinf(theta);
    GLfloat coords[] = {-1.0f + os, 1.0f, -1.0f + os, -1.0f, 1.0f + os, -1.0f, 1.0f + os, 1.0f};
    glClear(GL_COLOR_BUFFER_BIT);	
    glColor4f(red, green, 0.0f, 1.0f);
    glPushMatrix();
        glVertexPointer(2, GL_FLOAT, 0, coords);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glPopMatrix();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    theta += 0.01f;
}

@interface DemoView : NSOpenGLView
{
}
- (void)drawRect:(NSRect)rect;
@end

@implementation DemoView
- (void)drawRect:(NSRect)rect {
    [[self openGLContext] makeCurrentContext];
    render();
    [[self openGLContext] flushBuffer];
}
- (void)windowWillClose:(NSNotification *)notification {
    [NSApp terminate:self];
}
@end

static void setupWindow() {
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
    [myWindow makeKeyAndOrderFront: nil];
}

static void eventLoop() {
    double t = 0.0;
    const double dt = 1.0 / 60.0;
    double currentTime = CFAbsoluteTimeGetCurrent();
    double accum = 0.0;
    int quit = 0;
    render();
    while (!quit) {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        // Handle events (if any).
        NSEvent *event;
        while (event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES]) {
            switch ([event type]) {
                case NSLeftMouseDown:
                    quit = 1;
                    break;
                case NSLeftMouseUp:
                    break;
                case NSKeyDown:
                    red = 1.0f;
                    green = 0.0f;
                    break;
                case NSKeyUp:
                    red = 0.0f;
                    green = 1.0f;
                    break;
                default:
                    break;
            }
        }
                
        double newTime = CFAbsoluteTimeGetCurrent();
        double deltaTime = newTime - currentTime;
        currentTime = newTime;
        accum += deltaTime;

        render();
        while (accum >= dt) {
            t += dt;
            accum -= dt;
        }
        CGLFlushDrawable(fullScreenContextObj);
        [pool release];
    }
}

static void fullscreen() {
    CGLPixelFormatObj pixelFormatObj;
    GLint numPixelFormats;
        
    // Capture the main display
    CGDisplayCapture(kCGDirectMainDisplay);
        
    // Use the following to change resolution:
    /*
    int w = 1024;
    int h = 640;
    CFDictionaryRef refDisplayMode = CGDisplayBestModeForParameters(CGMainDisplayID(), 32, w, h, NULL);
    CGDisplaySwitchToMode(CGMainDisplayID(), refDisplayMode);
    */
        
    // Set up an array of attributes
    CGLPixelFormatAttribute attribs[] = {
        kCGLPFAFullScreen,
        kCGLPFADisplayMask, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
        
        // Attributes common to full-screen and non-fullscreen
        kCGLPFAAccelerated,
        kCGLPFANoRecovery,
        kCGLPFADoubleBuffer,
        kCGLPFAColorSize, 24,
        kCGLPFADepthSize, 16,
        0
    };
        
    CGLChoosePixelFormat(attribs, &pixelFormatObj, &numPixelFormats);
    // XXX Should use the following to share context with windowed mode:
    // CGLCreateContext(pixelFormatObj, [[openGLView openGLContext] CGLContextObj], &fullScreenContextObj);
    CGLCreateContext(pixelFormatObj, NULL, &fullScreenContextObj);
    CGLDestroyPixelFormat(pixelFormatObj);
        
    if (!fullScreenContextObj) {
        NSLog(@"Failed to create full-screen context");
        CGReleaseAllDisplays();
        return;
    }
        
    CGLSetCurrentContext(fullScreenContextObj);
    CGLSetFullScreen(fullScreenContextObj);
        
    // Lock us to the display's refresh rate (vsync).
    GLint newSwapInterval = 1;
    CGLSetParameter(fullScreenContextObj, kCGLCPSwapInterval, &newSwapInterval);

    //HideCursor();

    // OpenGL setup
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, CGDisplayPixelsWide(kCGDirectMainDisplay), CGDisplayPixelsHigh(kCGDirectMainDisplay));
    glOrtho(-10.0, 10.0, -10.0, 10.0, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnableClientState(GL_VERTEX_ARRAY);

    eventLoop();

    CGReleaseAllDisplays();
    [NSApp terminate:nil];
}

int main(int argc, char *argv[]) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSApp = [NSApplication sharedApplication];
    //setupWindow();
    //[NSApp run];
    fullscreen();
    eventLoop();
    [NSApp release];
    [pool release];
    return(0);
}
