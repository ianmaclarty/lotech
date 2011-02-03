#ifdef IOS
#ifndef LTIOSUTIL_H
#define LTIOSUTIL_H

bool ltIsIPad();
int ltIOSPortaitPixelWidth();
int ltIOSPortaitPixelHeight();
void ltNormalizeIOSTouchCoords(float x, float y, float *nx, float *ny);

// The caller should free the returned string with delete[].
const char *ltIOSBundlePath(const char *file);

#endif
#endif
