/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#define LT_CONCAT_(a, b) a##b
#define LT_CONCAT(a, b) LT_CONCAT_(a, b)
#define ct_assert(e) enum { LT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }

#define LT_PI 3.14159265358979323846f
#define LT_RADIANS_PER_DEGREE (LT_PI / 180.0f)
#define LT_DEGREES_PER_RADIAN (180.0f / LT_PI)

#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)

#define LT_INIT_DECL(module) \
    void module##_init();

#define LT_INIT_IMPL(module) \
    void module##_init() {}

LT_INIT_DECL(ltcommon)

typedef float           LTfloat;
typedef double          LTdouble;
typedef int             LTint;
typedef int             LTint32;
typedef char            LTbyte;
typedef short           LTshort;
typedef bool            LTbool;
typedef unsigned int    LTuint;
typedef unsigned short  LTushort;
typedef unsigned char   LTubyte;
typedef unsigned int    LTuint32;
typedef uintptr_t       LTuintptr;
typedef intptr_t        LTintptr;
typedef char*           LTstring;

typedef float           LTsecs;
typedef float           LTdegrees;

typedef LTuint32        LTpixel;
