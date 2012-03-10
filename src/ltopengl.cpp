    #ifdef LTGLES1
#define GLEXT(f) f##OES
#define GL_EXT(f) f##_OES
#else
#define GLEXT(f) f##EXT
#define GL_EXT(f) f##_EXT
#endif

#ifndef GL_DEPTH_COMPONENT16_EXT
#define GL_DEPTH_COMPONENT16_EXT GL_DEPTH_COMPONENT16
#endif

#include "ltopengl.h"

ct_assert(sizeof(GLfloat) == 4);

static char state_str[1024];
static void set_state_str();

//#define LTGLTRACE
#ifdef LTGLTRACE
#define trace {set_state_str(); ltLog("%s:%4d %-30s %s", __FILE__, __LINE__, __func__, state_str);}
#else
#define trace
#endif

#define LTGLCHECK
#ifdef LTGLCHECK
#define check_for_errors if (glGetError() != GL_NO_ERROR) \
    {ltLog("OpenGL error at %s:%d %s", __FILE__, __LINE__, __func__); ltAbort();}
#else
#define check_for_errors
#endif

// State
static bool texturing = false;
static bool texture_coord_arrays = false;
static LTBlendMode blend_mode = LT_BLEND_MODE_OFF;
static LTTextureMode texture_mode = LT_TEXTURE_MODE_MODULATE;
static bool depth_test = false;
static bool depth_mask = true;
static bool dither = false;
static bool alpha_test = false;
static bool stencil_test = false;
static bool vertex_arrays = false;
static bool index_arrays = false;
static bool color_arrays = false;
static bool fog = false;
static LTtexid bound_texture = 0;
static LTframebuf bound_framebuffer = 0;
static LTvertbuf bound_vertbuffer = 0;

void ltInitGLState() {
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_BLEND);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_DITHER);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_INDEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisable(GL_FOG);
    glBindTexture(GL_TEXTURE_2D, 0);
    GLEXT(glBindFramebuffer)(GL_EXT(GL_FRAMEBUFFER), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    check_for_errors
    trace
}

void ltEnableTexturing() {
    trace
    if (!texturing) {
        glEnable(GL_TEXTURE_2D);
        check_for_errors
        texturing = true;
    }
    trace
}

void ltDisableTexturing() {
    trace
    if (texturing) {
        glDisable(GL_TEXTURE_2D);
        check_for_errors
        texturing = false;
    }
    trace
}

void ltEnableTextureCoordArrays() {
    trace
    if (!texture_coord_arrays) {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        check_for_errors
        texture_coord_arrays = true;
    }
    trace
}

void ltDisableTextureCoordArrays() {
    trace
    if (texture_coord_arrays) {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        check_for_errors
        texture_coord_arrays = false;
    }
    trace
}

void ltTextureMode(LTTextureMode mode) {
    trace
    if (mode != texture_mode) {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
        check_for_errors
        texture_mode = mode;
    }
    trace
}

void ltTextureMagFilter(LTTextureFilter filter) {
    trace
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    check_for_errors
    trace
}

void ltTextureMinFilter(LTTextureFilter filter) {
    trace
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter); 
    check_for_errors
    trace
}

void ltBindTexture(LTtexid texture_id) {
    trace
    if (bound_texture != texture_id) {
        glBindTexture(GL_TEXTURE_2D, texture_id);
        check_for_errors
        bound_texture = texture_id;
    }
    trace
}

LTtexid ltGenTexture() {
    trace
    LTtexid t;
    glGenTextures(1, &t);
    check_for_errors
    trace
    return t;
}

void ltDeleteTexture(LTtexid texture_id) {
    trace
    if (bound_texture == texture_id) {
        ltBindTexture(0);
    }
    glDeleteTextures(1, &texture_id);
    check_for_errors
    trace
}

void ltTexImage(int width, int height, void *data) {
    trace
    #ifdef LTGLES1
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    #else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    #endif
    check_for_errors
    trace
}

void ltBlendMode(LTBlendMode new_mode) {
    trace
    LTBlendMode old_mode = blend_mode;
    if (old_mode != new_mode) {
        switch (new_mode) {
            case LT_BLEND_MODE_NORMAL:
                if (old_mode == LT_BLEND_MODE_OFF) {
                    glEnable(GL_BLEND);
                }
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case LT_BLEND_MODE_ADD:
                if (old_mode == LT_BLEND_MODE_OFF) {
                    glEnable(GL_BLEND);
                }
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                break;
            case LT_BLEND_MODE_COLOR:
                if (old_mode == LT_BLEND_MODE_OFF) {
                    glEnable(GL_BLEND);
                }
                glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
                break;
            case LT_BLEND_MODE_OFF:
                glDisable(GL_BLEND);
                break;
        }
        blend_mode = new_mode;
        check_for_errors
    }
    trace
}

void ltEnableDepthTest() {
    trace
    if (!depth_test) {
        glEnable(GL_DEPTH_TEST);
        check_for_errors
        depth_test = true;
    }
    trace
}

void ltDisableDepthTest() {
    trace
    if (depth_test) {
        glDisable(GL_DEPTH_TEST);
        check_for_errors
        depth_test = false;
    }
    trace
}

void ltEnableDepthMask() {
    trace
    if (!depth_mask) {
        glDepthMask(GL_TRUE);
        check_for_errors
        depth_mask = true;
    }
    trace
}

void ltDisableDepthMask() {
    trace
    if (depth_mask) {
        glDepthMask(GL_FALSE);
        check_for_errors
        depth_mask = false;
    }
    trace
}

void ltDepthFunc(LTDepthFunc f) {
    trace
    glDepthFunc(f);
    check_for_errors
    trace
}

void ltEnableDither() {
    trace
    if (!dither) {
        glEnable(GL_DITHER);
        check_for_errors
        dither = true;
    }
    trace
}

void ltDisableDither() {
    trace
    if (dither) {
        glDisable(GL_DITHER);
        check_for_errors
        dither = false;
    }
    trace
}

void ltEnableAlphaTest() {
    trace
    if (!alpha_test) {
        glEnable(GL_ALPHA_TEST);
        check_for_errors
        alpha_test = true;
    }
    trace
}

void ltDisableAlphaTest() {
    trace
    if (alpha_test) {
        glDisable(GL_ALPHA_TEST);
        check_for_errors
        alpha_test = false;
    }
    trace
}

void ltEnableStencilTest() {
    trace
    if (!stencil_test) {
        glEnable(GL_STENCIL_TEST);
        check_for_errors
        stencil_test = true;
    }
    trace
}

void ltDisableStencilTest() {
    trace
    if (stencil_test) {
        glDisable(GL_STENCIL_TEST);
        check_for_errors
        stencil_test = false;
    }
    trace
}

void ltEnableVertexArrays() {
    trace
    if (!vertex_arrays) {
        glEnableClientState(GL_VERTEX_ARRAY);
        check_for_errors
        vertex_arrays = true;
    }
    trace
}

void ltDisableVertexArrays() {
    trace
    if (vertex_arrays) {
        glDisableClientState(GL_VERTEX_ARRAY);
        check_for_errors
        vertex_arrays = false;
    }
    trace
}

void ltEnableIndexArrays() {
    trace
    if (!index_arrays) {
        glEnableClientState(GL_INDEX_ARRAY);
        check_for_errors
        index_arrays = true;
    }
    trace
}

void ltDisableIndexArrays() {
    trace
    if (index_arrays) {
        glDisableClientState(GL_INDEX_ARRAY);
        check_for_errors
        index_arrays = false;
    }
    trace
}

void ltEnableColorArrays() {
    trace
    if (!color_arrays) {
        glEnableClientState(GL_COLOR_ARRAY);
        check_for_errors
        color_arrays = true;
    }
    trace
}

void ltDisableColorArrays() {
    trace
    if (color_arrays) {
        glDisableClientState(GL_COLOR_ARRAY);
        check_for_errors
        color_arrays = false;
    }
    trace
}

void ltEnableFog() {
    trace
    if (!fog) {
        glEnable(GL_FOG);
        check_for_errors
        fog = true;
    }
    trace
}

void ltDisableFog() {
    trace
    if (fog) {
        glDisable(GL_FOG);
        check_for_errors
        fog = false;
    }
    trace
}

void ltFogColor(LTfloat r, LTfloat g, LTfloat b) {
    trace
    GLfloat colv[4];
    colv[0] = r;
    colv[1] = g;
    colv[2] = b;
    colv[3] = 1.0f;
    glFogfv(GL_FOG_COLOR, (const GLfloat*)colv);
    check_for_errors
    trace
}

void ltFogStart(LTfloat start) {
    trace
    glFogf(GL_FOG_START, start);
    check_for_errors
    trace
}

void ltFogEnd(LTfloat end) {
    trace
    glFogf(GL_FOG_END, end);
    check_for_errors
    trace
}

void ltFogMode(LTFogMode mode) {
    trace
    glFogf(GL_FOG_MODE, mode);
    check_for_errors
    trace
}

void ltClearColor(LTfloat r, LTfloat g, LTfloat b, LTfloat a) {
    trace
    glClearColor(r, g, b, a);
    check_for_errors
    trace
}

void ltClear(bool color, bool depthbuf) {
    trace
    GLbitfield clear_mask = 0;
    if (color) {
        clear_mask |= GL_COLOR_BUFFER_BIT;
    }
    if (depthbuf) {
        clear_mask |= GL_DEPTH_BUFFER_BIT;
    }
    glClear(clear_mask);
    check_for_errors
    trace
}

void ltColor(LTfloat r, LTfloat g, LTfloat b, LTfloat a) {
    trace
    glColor4f(r, g, b, a);
    check_for_errors
    trace
}

void ltMatrixMode(LTMatrixMode mode) {
    trace
    glMatrixMode(mode);
    check_for_errors
    trace
}

void ltPushMatrix() {
    trace
    glPushMatrix();
    check_for_errors
    trace
}

void ltPopMatrix() {
    trace
    glPopMatrix();
    check_for_errors
    trace
}

void ltMultMatrix(LTfloat *m) {
    trace
    glMultMatrixf(m);
    check_for_errors
    trace
}

void ltLoadIdentity() {
    trace
    glLoadIdentity();
    check_for_errors
    trace
}

void ltOrtho(LTfloat left, LTfloat right, LTfloat bottom, LTfloat top, LTfloat near, LTfloat far) {
    trace
    #ifdef LTGLES1
    glOrthof(left, right, bottom, top, near, far);
    #else
    glOrtho(left, right, bottom, top, near, far);
    #endif
    check_for_errors
    trace
}

void ltFrustum(LTfloat left, LTfloat right, LTfloat bottom, LTfloat top, LTfloat near, LTfloat far) {
    trace
    #ifdef LTGLES1
    glFrustumf(left, right, bottom, top, near, far);
    #else
    glFrustum(left, right, bottom, top, near, far);
    #endif
    check_for_errors
    trace
}

void ltTranslate(LTfloat x, LTfloat y, LTfloat z) {
    trace
    glTranslatef(x, y, z);
    check_for_errors
    trace
}

void ltRotate(LTdegrees degrees, LTfloat x, LTfloat y, LTfloat z) {
    trace
    glRotatef(degrees, x, y, z);
    check_for_errors
    trace
}

void ltScale(LTfloat x, LTfloat y, LTfloat z) {
    trace
    glScalef(x, y, z);
    check_for_errors
    trace
}

void ltViewport(int x, int y, int width, int height) {
    trace
    glViewport(x, y, width, height);
    check_for_errors
    trace
}

LTvertbuf ltGenVertBuffer() {
    trace
    LTvertbuf vb;
    glGenBuffers(1, &vb);
    check_for_errors
    trace
    return vb;
}

void ltBindVertBuffer(LTvertbuf vb) {
    trace
    if (bound_vertbuffer != vb) {
        glBindBuffer(GL_ARRAY_BUFFER, vb);
        check_for_errors
        bound_vertbuffer = vb;
    }
    trace
}

void ltDeleteVertBuffer(LTvertbuf vb) {
    trace
    // Make sure vb is not bound before deleting it.
    // This seems to fix an occasional crash on OSX.
    if (bound_vertbuffer == vb) {
        ltBindVertBuffer(0);
    }
    glDeleteBuffers(1, &vb);
    check_for_errors
    trace
}

void ltStaticVertBufferData(int size, const void *data) {
    trace
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    check_for_errors
    trace
}

void ltVertexPointer(int size, LTVertDataType type, int stride, void *data) {
    trace
    glVertexPointer(size, type, stride, data);
    check_for_errors
    trace
}

void ltColorPointer(int size, LTVertDataType type, int stride, void *data) {
    trace
    glColorPointer(size, type, stride, data);
    check_for_errors
    trace
}

void ltTexCoordPointer(int size, LTVertDataType type, int stride, void *data) {
    trace
    glTexCoordPointer(size, type, stride, data);
    check_for_errors
    trace
}

void ltDrawArrays(LTDrawMode mode, int start, int count) {
    trace
    glDrawArrays(mode, start, count);
    check_for_errors
    trace
}

void ltDrawElements(LTDrawMode mode, int n, LTvertindex *indices) {
    trace
    glDrawElements(mode, n, GL_UNSIGNED_SHORT, indices);
    check_for_errors
    trace
}

LTframebuf ltGenFramebuffer() {
    trace
    LTframebuf fb;
    GLEXT(glGenFramebuffers)(1, &fb);
    check_for_errors
    trace
    return fb;
}

void ltBindFramebuffer(LTframebuf fb) {
    trace
    if (bound_framebuffer != fb) {
        GLEXT(glBindFramebuffer)(GL_EXT(GL_FRAMEBUFFER), fb);
        check_for_errors
        bound_framebuffer = fb;
    }
    trace
}

void ltDeleteFramebuffer(LTframebuf fb) {
    trace
    if (bound_framebuffer == fb) {
        ltBindFramebuffer(0);
    }
    glDeleteFramebuffers(1, &fb);
    check_for_errors
    trace
}

void ltFramebufferTexture(LTtexid texture_id) {
    trace
    GLEXT(glFramebufferTexture2D)(GL_EXT(GL_FRAMEBUFFER), GL_EXT(GL_COLOR_ATTACHMENT0), GL_TEXTURE_2D, texture_id, 0);
    check_for_errors
    trace
}

bool ltFramebufferComplete() {
    GLenum status = GLEXT(glCheckFramebufferStatus)(GL_EXT(GL_FRAMEBUFFER));
    return status == GL_EXT(GL_FRAMEBUFFER_COMPLETE);
}

static void set_state_str() {
    const char *blend_mode_str = "?";
    switch (blend_mode) {
        case LT_BLEND_MODE_NORMAL:
            blend_mode_str = "N";
            break;
        case LT_BLEND_MODE_ADD:
            blend_mode_str = "A";
            break;
        case LT_BLEND_MODE_COLOR:
            blend_mode_str = "C";
            break;
        case LT_BLEND_MODE_OFF:
            blend_mode_str = "-";
            break;
    }

    const char *tex_mode_str = "?";
    switch (texture_mode) {
        case LT_TEXTURE_MODE_MODULATE:
            tex_mode_str = "M";
            break;
        case LT_TEXTURE_MODE_ADD:
            tex_mode_str = "A";
            break;
        case LT_TEXTURE_MODE_DECAL:
            tex_mode_str = "D";
            break;
        case LT_TEXTURE_MODE_BLEND:
            tex_mode_str = "B";
            break;
        case LT_TEXTURE_MODE_REPLACE:
            tex_mode_str = "R";
            break;
    }

    snprintf(state_str, 1024,
        "FB:%-3d TX:%-3d VB:%-3d BM:%s TM:%s %s %s %s %s %s %s %s %s %s %s %s",
        bound_framebuffer,
        bound_texture,
        bound_vertbuffer,
        blend_mode_str,
        tex_mode_str,
        texturing ? "TX" : "--",
        texture_coord_arrays ? "TA" : "--",
        vertex_arrays ? "VA" : "--",
        index_arrays ? "IA" : "--",
        color_arrays ? "CA" : "--",
        fog ? "FG" : "--",
        depth_test ? "DT" : "--",
        depth_mask ? "DM" : "--",
        dither ? "DH" : "--",
        alpha_test ? "AT" : "--",
        stencil_test ? "ST" : "--");
}
