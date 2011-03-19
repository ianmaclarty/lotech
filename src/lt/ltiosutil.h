#ifdef LTIOS
#ifndef LTIOSUTIL_H
#define LTIOSUTIL_H

bool ltIsIPad();
bool ltIsRetinaIPhone();
int ltIOSScreenWidth();
int ltIOSScreenHeight();
void ltNormalizeIOSTouchCoords(float x, float y, float *nx, float *ny);

// The caller should free the returned string with delete[].
// If suffix is not null, it is appended to the end of the returned
// string.
const char *ltIOSBundlePath(const char *file, const char *suffix);

#endif
#endif
