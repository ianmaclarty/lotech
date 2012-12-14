/* Copyright (C) 2010-2011 Ian MacLarty */

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <cfloat>
#include <list>
#include <set>
#include <map>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#if !defined(LTANDROID) && !defined(LTMINGW)
#include <glob.h>
#endif
#ifndef LTMINGW
#include <sys/socket.h>
#include <netinet/in.h>
#include <pwd.h>
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
#if defined(LTOSX) && !defined(LTNOGL)
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
#elif !defined(LTANDROID)
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

// Android specific
#ifdef LTANDROID
#include <android/asset_manager.h>
#include <android/log.h>
#endif

// OSX specific
#ifdef LTOSX
#import <AppKit/AppKit.h>
#endif

// iOS specific
#ifdef LTIOS
#import <objc/runtime.h>
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <GameKit/GameKit.h>
#endif

// Windows specific
#ifdef LTMINGW
#include <windows.h>
#endif

#if defined(LTDEVMODE) && (defined(LTOSX) || defined(LTLINUX) || defined(LTMINGW))
#define LTMEMTRACK 1
#endif

// Lotech.
#include "ltcommon.h"
#include "ltthreads.h"
#include "ltconfig.h"
#include "ltobject.h"
#include "ltffi.h"
#include "ltutil.h"
#include "ltopengl.h"
#include "ltinput.h"
#include "ltevent.h"
#include "ltaction.h"
#include "ltscene.h"
#include "ltimage.h"
#include "ltgraphics.h"
#include "ltpickle.h"
#include "lt3d.h"
#include "ltlua.h"
#include "ltnet.h"
#include "ltphysics.h"
#include "ltprotocol.h"
#include "ltrandom.h"
#include "ltresource.h"
#include "ltstate.h"
#include "lttime.h"
#include "lttween.h"
#include "ltaudio.h"
#include "ltmixer.h"
#include "ltvector.h"
#include "ltmesh.h"
#include "ltrendertarget.h"
#include "ltparticles.h"
#include "lttext.h"
#include "ltstore.h"
#include "ltfilestore.h"
#include "ltwavefront.h"
#include "ltlighting.h"

#include "ltgamecenter.h"
#include "ltios.h"
#include "ltiosutil.h"

#include "ltosx.h"
#include "ltosxutil.h"
