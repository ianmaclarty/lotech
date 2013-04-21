/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltgraphics)
#define LT_MAX_TEX_COORD 8192

struct LTPoint {
    LTfloat x;
    LTfloat y;

    LTPoint(LTfloat x, LTfloat y) {
        LTPoint::x = x;
        LTPoint::y = y;
    }

    LTPoint() {
        x = 0.0f;
        y = 0.0f;
    }

    void normalize() {
        if (x != 0.0f || y != 0.0f) {
            LTfloat l = len();
            x /= l;
            y /= l;
        }
    }

    LTfloat len() {
        return sqrtf(x * x + y * y);
    }
};

struct LTPoint3D {
    LTfloat x;
    LTfloat y;
    LTfloat z;

    LTPoint3D(LTfloat x, LTfloat y, LTfloat z) {
        LTPoint3D::x = x;
        LTPoint3D::y = y;
        LTPoint3D::z = z;
    }

    LTPoint3D() {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }

    void normalize() {
        LTfloat l = len();
        x /= l;
        y /= l;
        z /= l;
    }

    LTfloat len() {
        return sqrtf(x * x + y * y + z * z);
    }
};

struct LTCompactColor {
    LTubyte r;
    LTubyte g;
    LTubyte b;
    LTubyte a;

    LTCompactColor() {
        r = 255;
        g = 255;
        b = 255;
        a = 255;
    }

    LTCompactColor(LTubyte r, LTubyte g, LTubyte b, LTubyte a) {
        LTCompactColor::r = r;
        LTCompactColor::g = g;
        LTCompactColor::b = b;
        LTCompactColor::a = a;
    }
};

struct LTColor {
    LTfloat red;
    LTfloat green;
    LTfloat blue;
    LTfloat alpha;

    LTColor(LTfloat r, LTfloat g, LTfloat b, LTfloat a) {
        LTColor::red = r;
        LTColor::green = g;
        LTColor::blue = b;
        LTColor::alpha = a;
    }

    LTColor() {
        red = 1.0f;
        green = 1.0f;
        blue = 1.0f;
        alpha = 1.0f;
    }
};

enum LTDisplayOrientation {
    LT_DISPLAY_ORIENTATION_PORTRAIT,
    LT_DISPLAY_ORIENTATION_LANDSCAPE,
};

// Should be called before rendering each frame.
void ltInitGraphics();
void ltPrepareForRendering(
    int screen_viewport_x, int screen_viewport_y,
    int screen_viewport_width, int screen_viewport_height,
    LTfloat viewport_left, LTfloat viewport_bottom,
    LTfloat viewport_right, LTfloat viewport_top,
    LTColor *clear_color, bool clear_depthbuf);
void ltFinishRendering();

void ltSetMainFrameBuffer(LTframebuf fbo);

// The following functions should be called only once.
void ltSetViewPort(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2);
void ltSetScreenSize(int width, int height);
void ltSetDesignScreenSize(LTfloat width, LTfloat height);
void ltSetDisplayOrientation(LTDisplayOrientation orientation);
void ltAdjustViewportAspectRatio();

// This should be called whenever the screen is resized.
void ltResizeScreen(int width, int height);

// Dimensions of a pixel in viewport coords.
LTfloat ltGetPixelWidth();
LTfloat ltGetPixelHeight();

// Convert screen coords to world coords.
LTfloat ltGetViewPortX(LTfloat screen_x);
LTfloat ltGetViewPortY(LTfloat screen_y);

LTfloat ltGetViewPortLeftEdge();
LTfloat ltGetViewPortRightEdge();
LTfloat ltGetViewPortBottomEdge();
LTfloat ltGetViewPortTopEdge();

LTDisplayOrientation ltGetDisplayOrientation();

void ltPushPerspective(LTfloat nearz, LTfloat origin, LTfloat farz,
    LTfloat vanish_x, LTfloat vanish_y);
void ltPopPerspective();

void ltPushTint(LTfloat r, LTfloat g, LTfloat b, LTfloat a);
void ltPopTint();
void ltPeekTint(LTColor *color);
void ltRestoreTint();
void ltPushBlendMode(LTBlendMode mode);
void ltPopBlendMode();
void ltPushTextureMode(LTTextureMode mode);
void ltPopTextureMode();

void ltDrawUnitSquare();
void ltDrawUnitCircle();
void ltDrawRect(LTfloat x1, LTfloat y1, LTfloat x2, LTfloat y2);
void ltDrawEllipse(LTfloat x, LTfloat y, LTfloat rx, LTfloat ry);
void ltDrawPoly(LTfloat *vertices, int num_vertices); /* Must be convex */

void ltDrawConnectingOverlay();

void ltDrawAdBackground();
