/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltcommon)

ct_assert(sizeof(int) == 4);
ct_assert(sizeof(LTint) == 4);
ct_assert(sizeof(LTint32) == 4);
ct_assert(sizeof(LTubyte) == 1);
ct_assert(sizeof(LTbyte) == 1);
ct_assert(sizeof(LTushort) == 2);
ct_assert(sizeof(LTshort) == 2);
ct_assert(sizeof(LTuint32) == 4);
ct_assert(sizeof(LTfloat) == 4);
ct_assert(sizeof(float) == 4);
ct_assert(sizeof(LTdouble) == 8);
ct_assert(sizeof(LTuintptr) == sizeof(void*));
ct_assert(sizeof(LTintptr) == sizeof(void*));
