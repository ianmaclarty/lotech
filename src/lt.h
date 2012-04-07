/* Copyright (C) 2010-2011 Ian MacLarty */

#include <string.h>
#include <errno.h>
#include <cfloat>
#include <list>
#include <map>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#if !defined(LTANDROID) && !defined(LTMINGW)
#include <glob.h>
#endif

// OpenGL
#ifdef LTLINUX
    #define GLEW_STATIC 1
    #include <GL/glew.h>
    #include <GL/glfw.h>
#endif
#ifdef LTMINGW
    #define GLEW_STATIC 1
    #include <GL/glew.h>
    #include <GL/glfw.h>
#endif
#ifdef LTIOS
    #include <OpenGLES/ES1/gl.h>
    #include <OpenGLES/ES1/glext.h>
#endif
#ifdef LTOSX
    //#include <OpenGL/GL.h>
    #define GLEW_STATIC 1
    #include <GL/glew.h>
    #include <GL/glfw.h>
#endif
#ifdef LTANDROID
    #define GL_GLEXT_PROTOTYPES 1
    #include <GLES/gl.h>
    #include <GLES/glext.h>
#endif

// OpenAL
#ifdef LTLINUX
#include <AL/al.h>
#include <AL/alc.h>
#elif LTMINGW
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

// Box2D
#include "Box2D/Box2D.h"

// libPNG
extern "C" {
#include "png.h"
#include "pngconf.h"
}

// Lua.
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// Android specific headers.
#ifdef LTANDROID
#include <android/asset_manager.h>
#include <android/log.h>
#endif

// OSX specific headers.
#ifdef LTOSX
#import <AppKit/AppKit.h>
#endif

#ifdef LTIOS
#import <objc/runtime.h>
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <GameKit/GameKit.h>
#endif

// Lotech.
#include "ltcommon.h"
#include "ltobject.h"
#include "ltopengl.h"
#include "ltevent.h"
#include "ltscene.h"
#include "ltimage.h"
#include "ltgraphics.h"
#include "ltpickle.h"
#include "lt3d.h"
#include "ltinput.h"
#include "ltlua.h"
#include "ltnet.h"
#include "ltphysics.h"
#include "ltprotocol.h"
#include "ltrandom.h"
#include "ltresource.h"
#include "ltstate.h"
#include "lttime.h"
#include "lttween.h"
#include "ltutil.h"
#include "ltaudio.h"
#include "ltvector.h"
#include "ltrendertarget.h"
#include "ltparticles.h"
#include "lttext.h"
#include "ltstore.h"

#include "ltgamecenter.h"
#include "ltios.h"
#include "ltiosutil.h"

#include "ltosx.h"
#include "ltosxutil.h"
