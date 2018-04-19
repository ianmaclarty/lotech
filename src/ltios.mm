/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#ifdef LTIOS
#include "lt.h"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#import <AudioToolbox/AudioServices.h>
#import <CoreMotion/CoreMotion.h>

#define MAX_TOUCHES 10
static UITouch *active_touches[MAX_TOUCHES];

int ltIOSMain(int argc, char *argv[])
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    int retVal = UIApplicationMain(argc, argv, nil, @"LTAppDelegate");
    [pool release];
    return retVal;
}

int main(int argc, char *argv[]) {
    return ltIOSMain(argc, argv);
}

@class LTViewController;
@class LTView;
static LTViewController *lt_view_controller = nil;
static LTView *lt_view = nil;

static void set_audio_category() {
    UInt32 category = kAudioSessionCategory_AmbientSound;
    AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
}

static void audio_interrupt(void *ud, UInt32 state) {
    if (state == kAudioSessionBeginInterruption) {
        AudioSessionSetActive(NO);
        ltAudioSuspend(); 
    } else if (state == kAudioSessionEndInterruption) {
        set_audio_category();
        AudioSessionSetActive(YES);
        ltAudioResume(); 
    }
}

static void init_touches() {
    for (int i = 0; i < MAX_TOUCHES; i++) {
        active_touches[i] = NULL;
    }
}

static int lookup_touch(UITouch *touch) {
    for (int i = 0; i < MAX_TOUCHES; i++) {
        if (active_touches[i] == touch) return i;
    }
    return -1;
}

static void begin_touch(UITouch *touch) {
    for (int i = 0; i < MAX_TOUCHES; i++) {
        if (active_touches[i] == NULL) {
            active_touches[i] = touch;
            return;
        }
    }
}

static void end_touch(UITouch *touch) {
    for (int i = 0; i < MAX_TOUCHES; i++) {
        if (active_touches[i] == touch) {
            active_touches[i] = NULL;
        }
    }
}

void ltIOSInit() {
    init_touches();
    // Turn off orientation change animations initially
    // to avoid an orientation change animation after loading.
    [UIView setAnimationsEnabled:NO];

    #ifdef LTDEVMODE
    ltClientInit();
    #endif

    AudioSessionInitialize(NULL, NULL, audio_interrupt, NULL);
    set_audio_category();
    AudioSessionSetActive(true);

    ltLuaSetup();
}

void ltIOSSetViewController(LTViewController *view_c) {
    lt_view_controller = view_c;
    [lt_view_controller retain];

    #ifdef LTGAMECENTER
    ltIOSInitGameCenter();
    #endif

    #ifdef LTADS 
    ltShowAds(LTADS);
    #endif
}

void ltIOSTeardown() {
    #ifdef LTGAMECENTER
    ltIOSTeardownGameCenter();
    #endif
    ltLuaTeardown();
    if (lt_view_controller != nil) {
        [lt_view_controller release];
    }
}

void ltIOSResize(int width, int height) {
    ltResizeScreen(width, height);
}

// This is used to decide whether rotations should
// be animated (we don't want to animate the initial
// rotation after app launch), as well as whether to
// disable rotations altogether when the app is using
// the accelerometer for input (we want to however
// allow the initial rotation).
static int frames_since_disable_animations = 0;

void ltIOSRender() {
#ifdef FPS
    static double t0 = 0.0;
    static int frame_count = 0;
#endif

    ltLuaAdvance(1.0f/60.0f);
    ltLuaRender();

    #ifdef LTDEVMODE
    ltClientStep();
    #endif

    if (frames_since_disable_animations == 60) {
        [UIView setAnimationsEnabled:YES];
    }
    frames_since_disable_animations++;

#ifdef FPS
    double t = ltIOSGetTime();
    if (t0 == 0.0) {
        t0 = t;
    } else {
        frame_count++;
        if (t - t0 > 2.0) {
            ltLog("FPS: %.2f", (double)frame_count / 2.0);
            frame_count = 0;
            t0 = t;
        }
    }
#endif
}

void ltIOSGarbageCollect() {
    ltLuaGarbageCollect();
}

void ltIOSTouchesBegan(NSSet *touches) {
    NSEnumerator *e = [touches objectEnumerator];
    UITouch *touch;
    while ((touch = [e nextObject])) {
        begin_touch(touch);
        CGPoint pos = [touch locationInView:touch.view];
        ltLuaTouchDown(lookup_touch(touch), pos.x, pos.y);
    }
}

void ltIOSTouchesMoved(NSSet *touches) {
    NSEnumerator *e = [touches objectEnumerator];
    UITouch *touch;
    while ((touch = [e nextObject])) {
        CGPoint pos = [touch locationInView:touch.view];
        ltLuaTouchMove(lookup_touch(touch), pos.x, pos.y);
    }
}

void ltIOSTouchesEnded(NSSet *touches) {
    NSEnumerator *e = [touches objectEnumerator];
    UITouch *touch;
    while ((touch = [e nextObject])) {
        CGPoint pos = [touch locationInView:touch.view];
        ltLuaTouchUp(lookup_touch(touch), pos.x, pos.y);
        end_touch(touch);
    }
}

void ltIOSTouchesCancelled(NSSet *touches) {
    ltIOSTouchesEnded(touches);
}

UIViewController *ltIOSGetViewController() {
    return (UIViewController*)lt_view_controller;
}

void ltIOSSaveState() {
    ltSaveState();
    ltIOSSyncStore();
}

void ltIOSResignActive() {
#ifdef LTDEVMODE
    ltClientShutdown();
#endif
}

void ltIOSBecomeActive() {
#ifdef LTDEVMODE
    ltClientInit();
#endif
    frames_since_disable_animations = 0;
}





@interface LTView : UIView {    
@private
    EAGLContext *glContext;
    
    GLint backingWidth;
    GLint backingHeight;
    
    GLuint defaultFramebuffer, colorRenderbuffer, depthRenderbuffer;

    BOOL animating;
    NSInteger animationFrameInterval;
    id displayLink;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;

- (void) startAnimation;
- (void) stopAnimation;
- (void) drawView:(id)sender;

@end

@implementation LTView

@synthesize animating;
@dynamic animationFrameInterval;

+ (Class) layerClass {
    return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame {    
    if ((self = [super initWithFrame:frame])) {
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        
        depthRenderbuffer = 0;

        glContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
            
        if (!glContext || ![EAGLContext setCurrentContext:glContext]) {
            return nil;
        }
            
        glGenFramebuffersOES(1, &defaultFramebuffer);
        ltSetMainFrameBuffer(defaultFramebuffer);
        glGenRenderbuffersOES(1, &colorRenderbuffer);
        glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, colorRenderbuffer);
        
        animating = FALSE;
        animationFrameInterval = 1;
        displayLink = nil;

        // Needed for retina display.
        if([[UIScreen mainScreen] respondsToSelector: NSSelectorFromString(@"scale")]) {
            if([self respondsToSelector: NSSelectorFromString(@"contentScaleFactor")]) {
                self.contentScaleFactor = [[UIScreen mainScreen] scale];
            }
        }
        
        [self setMultipleTouchEnabled:YES]; 
    }
    
    return self;
}

- (void) drawView:(id)sender {
    //fprintf(stderr, "render\n");
    ltIOSRender();
    
    /*
    glEnableClientState(GL_VERTEX_ARRAY);
    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity();
    glViewport(0, 0, backingWidth, backingHeight);
    glOrthof(0.0f, 6.0f, 0.0f, 9.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    GLfloat vertices[8];
    GLfloat x1 = 1.0f;
    GLfloat y1 = 1.0f;
    GLfloat x2 = 3.0f;
    GLfloat y2 = 3.0f;
    vertices[0] = x1;
    vertices[1] = y1;
    vertices[2] = x2;
    vertices[3] = y1;
    vertices[4] = x2;
    vertices[5] = y2;
    vertices[6] = x1;
    vertices[7] = y2;
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    */
    
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
    [glContext presentRenderbuffer:GL_RENDERBUFFER_OES];
}

- (void) layoutSubviews {
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
    [glContext renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
    
#ifdef LTDEPTHBUF
    if (depthRenderbuffer) {
        glDeleteRenderbuffersOES(1, &depthRenderbuffer);
    }
    glGenRenderbuffersOES(1, &depthRenderbuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
    glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT24_OES, backingWidth, backingHeight);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
#endif
    
    if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
        NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return;
    }
    
    ltIOSResize(backingWidth, backingHeight);
    
    [self drawView:nil];
}

- (NSInteger) animationFrameInterval {
    return animationFrameInterval;
}

- (void) setAnimationFrameInterval:(NSInteger)frameInterval {
    if (frameInterval >= 1) {
        animationFrameInterval = frameInterval;
        if (animating) {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void) startAnimation {
    if (!animating) {
        displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
        [displayLink setFrameInterval:animationFrameInterval];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        
        animating = TRUE;
    }
}

- (void)stopAnimation {
    if (animating) {
        [displayLink invalidate];
        displayLink = nil;
        
        animating = FALSE;
    }
}

- (void) dealloc {
    ltIOSTeardown();
    
    // Tear down GL
#ifdef LTDEPTHBUF
    if (depthRenderbuffer) {
        glDeleteRenderbuffersOES(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
#endif
    
    if (defaultFramebuffer) {
        glDeleteFramebuffersOES(1, &defaultFramebuffer);
        defaultFramebuffer = 0;
    }
    
    if (colorRenderbuffer) {
        glDeleteRenderbuffersOES(1, &colorRenderbuffer);
        colorRenderbuffer = 0;
    }
    
    // Tear down context
    if ([EAGLContext currentContext] == glContext) {
        [EAGLContext setCurrentContext:nil];
    }
    
    [glContext release];
    glContext = nil;

    
    [super dealloc];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    ltIOSTouchesBegan(touches);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{    
    ltIOSTouchesMoved(touches);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{    
    ltIOSTouchesEnded(touches);
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    ltIOSTouchesCancelled(touches);
}

@end


static CMMotionManager *motionManager = nil;


@interface LTViewController : UIViewController { }
@end

static UIInterfaceOrientation last_orientation = UIInterfaceOrientationPortrait;
static bool has_rotated = false;

static BOOL handle_orientation(UIInterfaceOrientation orientation) {
    BOOL res = NO;
    if (has_rotated && motionManager != nil && frames_since_disable_animations > 60) {
        // Prevent screen rotation if using motion input.
        return orientation == last_orientation;
    }
    switch (ltGetDisplayOrientation()) {
        case LT_DISPLAY_ORIENTATION_PORTRAIT:
            res = (orientation == UIInterfaceOrientationPortrait
                    || orientation == UIInterfaceOrientationPortraitUpsideDown);
            break;
        case LT_DISPLAY_ORIENTATION_LANDSCAPE:
            res = (orientation == UIInterfaceOrientationLandscapeRight
                    || orientation == UIInterfaceOrientationLandscapeLeft);
            break;
    }
    if (res == YES) {
        has_rotated = true;
        last_orientation = orientation;
    }
    return res;
}

@implementation LTViewController

- (void)loadView {
    [super loadView];
    ltIOSInit();
    CGRect screen_bounds = [[UIScreen mainScreen] bounds];
    CGFloat screen_w = screen_bounds.size.width;
    CGFloat screen_h = screen_bounds.size.height;
    switch (ltGetDisplayOrientation()) {
        case LT_DISPLAY_ORIENTATION_PORTRAIT:
            if (screen_w > screen_h) {
                CGFloat tmp = screen_w;
                screen_w = screen_h;
                screen_h = tmp;
            }
            break;
        case LT_DISPLAY_ORIENTATION_LANDSCAPE:
            if (screen_w < screen_h) {
                CGFloat tmp = screen_w;
                screen_w = screen_h;
                screen_h = tmp;
            }
            break;
    }
    CGRect frame = CGRectMake(0, 0, screen_w, screen_h);
    lt_view = [[[LTView alloc] initWithFrame:frame] autorelease];
    [[self view] addSubview: lt_view];
}

- (void)dealloc {
    [super dealloc];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    ltIOSGarbageCollect();
}

- (void)viewDidLoad
{
}

- (void)viewDidUnload
{
    [super viewDidUnload];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)orientation
{
    return handle_orientation(orientation);
}

- (BOOL)shouldAutorotate {
    /*
    UIInterfaceOrientation orientation = [[UIDevice currentDevice] orientation];
    return handle_orientation(orientation);
    */
    return YES;
}

-(NSUInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscapeRight | UIInterfaceOrientationMaskLandscapeLeft;
}

-(BOOL)prefersStatusBarHidden
{
    return YES;
}

@end


static UIWindow *window;
static LTViewController *viewController;

@interface LTAppDelegate : NSObject <UIApplicationDelegate> { }
@end

@implementation LTAppDelegate
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
    [window retain];
    viewController = [[[LTViewController alloc] init] autorelease];
    [viewController retain];
    [window addSubview:viewController.view];
    window.rootViewController = viewController;
    [window makeKeyAndVisible];
    ltIOSSetViewController(viewController);
    return YES;
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    ltIOSSaveState();
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    ltIOSResignActive();
    [lt_view stopAnimation];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    ltIOSBecomeActive();
    [lt_view startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
}

- (void)dealloc
{
    [super dealloc];
    if (motionManager != nil) {
        [motionManager release];
    }
}

@end

void ltIOSSampleAccelerometer(LTdouble *x, LTdouble *y, LTdouble *z) {
    *x = 0.0;
    *y = 0.0;
    *z = 0.0;
    if (motionManager == nil) {
        motionManager = [[CMMotionManager alloc] init];
        [[UIApplication sharedApplication] setIdleTimerDisabled: YES];
    }
    if (!motionManager.accelerometerAvailable) return;
    if (!motionManager.accelerometerActive) {
        [motionManager startAccelerometerUpdates];
    }
    CMAccelerometerData *data = motionManager.accelerometerData;
    if (data == nil) return;
    CMAcceleration accel = data.acceleration;

    *z = accel.z;

    if (has_rotated) {
        switch (last_orientation) {
            case UIInterfaceOrientationPortrait:
                *x = accel.x;
                *y = accel.y;
                break;
            case UIInterfaceOrientationPortraitUpsideDown:
                *x = -accel.x;
                *y = -accel.y;
                break;
            case UIInterfaceOrientationLandscapeLeft:
                *x = accel.y;
                *y = -accel.x;
                break;
            case UIInterfaceOrientationLandscapeRight:
                *x = -accel.y;
                *y = accel.x;
                break;
            case UIInterfaceOrientationUnknown:
                break;
        }
    } else {
        *x = accel.x;
        *y = accel.y;
    }
}

#endif // LTIOS
