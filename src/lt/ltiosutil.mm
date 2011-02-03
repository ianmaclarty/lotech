#import <UIKit/UIKit.h>

#include "ltiosutil.h"

static float scaling() {
    float scale = 1.0f;
    if([[UIScreen mainScreen] respondsToSelector: NSSelectorFromString(@"scale")]) {
        scale = [[UIScreen mainScreen] scale];
    }
    return scale;
}

bool ltIsIPad() {
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
        return true;
    } else {
        return false;
    }
}

int ltIOSPortaitPixelWidth() {
    float scale = scaling();
    if (ltIsIPad()) {
        return (int)(768.0f * scale);
    } else {
        return (int)(320.0f * scale);
    }
}

int ltIOSPortaitPixelHeight() {
    float scale = scaling();
    if (ltIsIPad()) {
        return (int)(1024.0f * scale);
    } else {
        return (int)(480.0f * scale);
    }
}

void ltNormalizeIOSTouchCoords(float x, float y, float *nx, float *ny) {
    if (ltIsIPad()) {
        *nx = (x / 768.0f) * 2.0f - 1.0f;
        *ny = (1.0f - (y / 1024.0f)) * 2.0f - 1.0f;
    } else {
        *nx = (x / 320.0f) * 2.0f - 1.0f;
        *ny = (1.0f - (y / 480.0f)) * 2.0f - 1.0f;
    }
}

const char *ltIOSBundlePath(const char *file) {
    const char *dir = [[[NSBundle mainBundle] bundlePath] UTF8String];
    char *path = new char[strlen(dir) + strlen(file) + 2];
    sprintf(path, "%s/%s", dir, file);
    return path;
}
