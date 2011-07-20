#ifdef LTADS
#import <iAd/iAD.h>
#import "GADBannerView.h"
#import "GADBannerViewDelegate.h"

#include "ltads.h"
#include "ltcommon.h"
#include "ltios.h"
#include "ltiosutil.h"
#include "ltgraphics.h"
#include "ltlua.h"

#define ADMOB_REFRESH_RATE 30.0

static bool user_is_interacting_with_ad = false;
static ADBannerView *iAd_view = nil;
static bool iAd_is_visible = true;
static bool admob_is_visible = true;
static GADBannerView *admob_view = nil;
static LTAdPlacement placement = LT_AD_TOP;

static bool iAd_available();
static void init_iAd();
static void show_iAd();
static void hide_iAd();

static void init_admob(const char *pubid);
static void load_admob_request();
static void init_admob_timer();
static void show_admob();
static void hide_admob();

void ltShowAds(LTAdPlacement p) {
    placement = p;
    // We default to iAd and use admob as a fallback.
    init_iAd();
    if (ltIsIPad()) {
        #ifdef LTADMOB_IPAD_PUBID
        init_admob(STR(LTADMOB_IPAD_PUBID));
        #endif
    } else {
        #ifdef LTADMOB_IPHONE_PUBID
        init_admob(STR(LTADMOB_IPHONE_PUBID));
        #endif
    }
    if (iAd_view == nil) {
        // Start a timer to auto-refresh admob if iAd not available
        // (if iAd available, we use iAd load failures to trigger admob loads).
        load_admob_request();
        init_admob_timer();
    }
}

@interface IADDelegate : NSObject<ADBannerViewDelegate>
- (void)bannerView:(ADBannerView *)banner didFailToReceiveAdWithError:(NSError *)error;
- (void)bannerViewDidLoadAd:(ADBannerView *)banner;
- (BOOL)bannerViewActionShouldBegin:(ADBannerView *)banner willLeaveApplication:(BOOL)willLeave;
- (void)bannerViewActionDidFinish:(ADBannerView *)banner;
@end

@implementation IADDelegate
- (void)bannerView:(ADBannerView *)banner didFailToReceiveAdWithError:(NSError *)error {
    hide_iAd();
    load_admob_request();
}
- (void)bannerViewDidLoadAd:(ADBannerView *)banner {
    hide_admob();
    show_iAd();
}
- (BOOL)bannerViewActionShouldBegin:(ADBannerView *)banner willLeaveApplication:(BOOL)willLeave {
    if (!willLeave) {
        user_is_interacting_with_ad = true;
        ltLuaSuspend();
    }
    return YES;
}
- (void)bannerViewActionDidFinish:(ADBannerView *)banner {
    user_is_interacting_with_ad = false;
    ltLuaResume();
}
@end

static void init_iAd() {
    if (iAd_available()) {
        UIViewController *vc = ltIOSGetViewController();
        if (vc != nil) {
            LTDisplayOrientation orientation = ltGetDisplayOrientation();
            CGRect rect;
            if (placement == LT_AD_TOP) {
                if (ltIsIPad()) {
                    rect = CGRectMake(0, 12, 0, 0);
                } else {
                    if (orientation == LT_DISPLAY_ORIENTATION_PORTRAIT) {
                        rect = CGRectMake(0, 0, 0, 0);
                    } else {
                        rect = CGRectMake(0, 9, 0, 0);
                    }
                }
            } else {
                if (ltIsIPad()) {
                    if (orientation == LT_DISPLAY_ORIENTATION_PORTRAIT) {
                        rect = CGRectMake(0, 1024-78, 0, 0);
                    } else {
                        rect = CGRectMake(0, 768-78, 0, 0);
                    }
                } else {
                    if (orientation == LT_DISPLAY_ORIENTATION_PORTRAIT) {
                        rect = CGRectMake(0, 480-50, 0, 0);
                    } else {
                        rect = CGRectMake(0, 320-41, 0, 0);
                    }
                }
            }
            iAd_view = [[ADBannerView alloc] initWithFrame:rect];
            iAd_view.delegate = [[IADDelegate alloc] init];
            if (orientation == LT_DISPLAY_ORIENTATION_PORTRAIT) {
                iAd_view.currentContentSizeIdentifier = ADBannerContentSizeIdentifierPortrait;
            } else {
                iAd_view.currentContentSizeIdentifier = ADBannerContentSizeIdentifierLandscape;
            }
            [vc.view addSubview:iAd_view];
            hide_iAd();
        }
    } else {
        iAd_is_visible = false;
    }
}

static void show_iAd() {
    if (iAd_view != nil && !iAd_is_visible) {
        if (placement == LT_AD_TOP) {
            iAd_view.frame = CGRectOffset(iAd_view.frame, 0, 200);
        } else {
            iAd_view.frame = CGRectOffset(iAd_view.frame, 0, -200);
        }
        UIViewController *vc = ltIOSGetViewController();
        [vc.view bringSubviewToFront:iAd_view];
        iAd_is_visible = true;
    }
}

static void hide_iAd() {
    if (iAd_view != nil && iAd_is_visible) {
        if (placement == LT_AD_TOP) {
            iAd_view.frame = CGRectOffset(iAd_view.frame, 0, -200);
        } else {
            iAd_view.frame = CGRectOffset(iAd_view.frame, 0, 200);
        }
        /*      
        [UIView beginAnimations:@"animateAdBannerOff" context:NULL];
        banner.frame = CGRectOffset(banner.frame, 0, -50);
        [UIView commitAnimations];
        */
        iAd_is_visible = false;
    }
}

static bool iAd_available() {
    Class iAdClass = (NSClassFromString(@"ADBannerView"));
    // The device must be running running iOS 4.0 or later.
    NSString *reqSysVer = @"4.0";
    NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
    BOOL osVersionSupported = ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending);
    if (!(iAdClass && osVersionSupported)) {
        return false;
    } else {
        return true;
    }
}

//----------------------------------------------------------------

@interface AdmobLoader : NSObject
-(void)loadRequest:(NSTimer*)timer;
@end

@implementation AdmobLoader
-(void)loadRequest:(NSTimer*)timer {
    load_admob_request();
}
@end

@interface AdmobDelegate : NSObject <GADBannerViewDelegate>
- (void)adViewDidReceiveAd:(GADBannerView *)bannerView;
- (void)adViewWillPresentScreen:(GADBannerView *)bannerView;
- (void)adViewWillDismissScreen:(GADBannerView *)bannerView;
- (void)adViewWillLeaveApplication:(GADBannerView *)bannerView;
@end

@implementation AdmobDelegate
- (void)adViewDidReceiveAd:(GADBannerView *)bannerView {
    show_admob();
}
- (void)adViewWillPresentScreen:(GADBannerView *)bannerView {
    user_is_interacting_with_ad = true;
    ltLuaSuspend();
}
- (void)adViewWillDismissScreen:(GADBannerView *)bannerView {
    user_is_interacting_with_ad = false;
    ltLuaResume();
}
- (void)adViewWillLeaveApplication:(GADBannerView *)bannerView {
    user_is_interacting_with_ad = false;
    ltLuaResume();
}
@end

static void init_admob_timer() {
    AdmobLoader *admob_loader = [[AdmobLoader alloc] init];
    [NSTimer scheduledTimerWithTimeInterval:ADMOB_REFRESH_RATE
        target:admob_loader
        selector:@selector(loadRequest:)
        userInfo:nil
        repeats:YES];
}

static void init_admob(const char *pubid) {
    if (admob_view == nil) {
        LTDisplayOrientation orientation = ltGetDisplayOrientation();
        UIViewController *vc = ltIOSGetViewController();
        CGRect rect;
        if (ltIsIPad()) {
            if (orientation == LT_DISPLAY_ORIENTATION_PORTRAIT) {
                if (placement == LT_AD_TOP) {
                    rect = CGRectMake(20, 0, 728, 90);
                } else {
                    rect = CGRectMake(20, 1024-90, 728, 90);
                }
            } else {
                if (placement == LT_AD_TOP) {
                    rect = CGRectMake(148, 0, 728, 90);
                } else {
                    rect = CGRectMake(148, 768-90, 728, 90);
                }
            }
        } else {
            if (orientation == LT_DISPLAY_ORIENTATION_PORTRAIT) {
                if (placement == LT_AD_TOP) {
                    rect = CGRectMake(0, 0, 320, 50);
                } else {
                    rect = CGRectMake(0, 480-50, 320, 50);
                }
            } else {
                if (placement == LT_AD_TOP) {
                    rect = CGRectMake(80, 0, 320, 50);
                } else {
                    rect = CGRectMake(80, 320-50, 320, 50);
                }
            }
        }
        admob_view = [[GADBannerView alloc] initWithFrame:rect];
        admob_view.delegate = [[AdmobDelegate alloc] init];
        admob_view.adUnitID = [NSString stringWithUTF8String:pubid];
        admob_view.rootViewController = vc;
        [vc.view addSubview:admob_view];

        hide_admob();
    }
}

static void load_admob_request() {
    if (admob_view != nil) {
        GADRequest *request = [GADRequest request];
        request.testDevices = [NSArray arrayWithObjects:
            GAD_SIMULATOR_ID,                               // Simulator
            @"e75cf18a1530dc79d2a1b2c8772df100b00b94b3",    // Ian iPhone 3GS
            @"cd77d54f2e974c814605f3df420ed29a4afb1eb3",    // Ian iPad2
            @"f482fa2f0b86d289a49dc31fce1a376acbbc2b42",    // Bethany iPod
            @"7ace612cfe8d58fbeaf6de1aa3da4597dee5cafd",    // Jon iPhone 4
            nil];
        [admob_view loadRequest:request];
    }
}

static void show_admob() {
    if (admob_view != nil && !admob_is_visible && !iAd_is_visible) {
        if (placement == LT_AD_TOP) {
            admob_view.frame = CGRectOffset(admob_view.frame, 0, 200);
        } else {
            admob_view.frame = CGRectOffset(admob_view.frame, 0, -200);
        }
        UIViewController *vc = ltIOSGetViewController();
        [vc.view bringSubviewToFront:admob_view];
        admob_is_visible = true;
    }
}

static void hide_admob() {
    if (admob_view != nil && admob_is_visible) {
        if (placement == LT_AD_TOP) {
            admob_view.frame = CGRectOffset(admob_view.frame, 0, -200);
        } else {
            admob_view.frame = CGRectOffset(admob_view.frame, 0, 200);
        }
        admob_is_visible = false;
    }
}
#endif //LTADS
