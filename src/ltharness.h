/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTHARNESS_H
#define LTHARNESS_H

#include "ltcommon.h"
#include "ltinput.h"

typedef void (*ltVoidCallback)(void);
typedef void (*ltKeyCallback)(LTKey);
typedef void (*ltPointerCallback)(int, LTfloat, LTfloat);
typedef void (*ltPairCallback)(LTfloat, LTfloat);

void ltHarnessInit(bool fullscreen, const char *title, int fps,
    ltVoidCallback setup, ltVoidCallback teardown,
    ltVoidCallback render, ltVoidCallback advance,
    ltKeyCallback keyDown, ltKeyCallback keyUp,
    ltPointerCallback mouseDown, ltPointerCallback mouseUp,
    ltPointerCallback mouseMove,
    ltPairCallback resizeWindow);

void ltHarnessQuit();

#endif
