#include "ltcommon.h"
#include "ltgraphics.h"
#include "ltiosutil.h"
#include "ltlua.h"
#include "ltprotocol.h"

// The following is required for converting UITouch objects to input_ids.
ct_assert(sizeof(UITouch*) == sizeof(int));

#ifdef LTDEPTHBUF
    GLuint depth_buf;
#endif

void ltIOSInit() {
    #ifdef LTDEVMODE
    ltClientInit();
    #endif

    ltSetScreenSize(ltIOSPortaitPixelWidth(), ltIOSPortaitPixelHeight());

    #ifdef LTDEPTHBUF
    glGenRenderbuffersOES(1, &depth_buf);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, depth_buf);
    glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, ltIOSPortaitPixelWidth(), ltIOSPortaitPixelHeight());
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depth_buf);
    #endif

    const char *path = ltIOSBundlePath("main", ".lua");
    ltLuaSetup(path);
    delete[] path;
}

void ltIOSTeardown() {
    ltLuaTeardown();

    #ifdef LTDEPTHBUF
    glDeleteRenderbuffersOES(1, &depth_buf);
    #endif
}

void ltIOSRender() {
    ltLuaRender();
    ltLuaAdvance();

    #ifdef LTDEVMODE
    ltClientStep();
    #endif
}

void ltIOSGarbageCollect() {
    ltLuaGarbageCollect();
}

void ltIOSTouchesBegan(NSSet *touches) {
    ltLuaPointerDown(0, 0.0f, 0.0f);
}
void ltIOSTouchesMoved(NSSet *touches) {
}
void ltIOSTouchesEnded(NSSet *touches) {
}
void ltIOSTouchesCancelled(NSSet *touches) {
    ltIOSTouchesEnded(touches);
}
