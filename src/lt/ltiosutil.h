#ifdef IOS
#ifndef IOSUTIL_H
#define IOSUTIL_H

bool ltIsIPad();
int ltIOSPortaitPixelWidth();
int ltIOSPortaitPixelHeight();
void ltNormalizeIOSTouchCoords(float x, float y, float *nx, float *ny);

#endif
#endif
