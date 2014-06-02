/*****************************************************************************
Copyright (C) 2010-2013 Ian MacLarty

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*****************************************************************************/

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
#include <stdint.h>
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
#endif
#ifdef LTMINGW
    #define GLEW_STATIC 1
    #include <GL/glew.h>
#endif
#ifdef LTIOS
    #include <OpenGLES/ES1/gl.h>
    #include <OpenGLES/ES1/glext.h>
#endif
#if defined(LTOSX) && !defined(LTNOGL)
    //#include <OpenGL/GL.h>
    #define GLEW_STATIC 1
    #include <GL/glew.h>
#endif
#ifdef LTANDROID
    #define GL_GLEXT_PROTOTYPES 1
    #include <GLES/gl.h>
    #include <GLES/glext.h>
#endif
#ifdef LTTIZEN
#include <FGraphicsOpengl.h>
using namespace Tizen::Graphics::Opengl;
#endif

// OpenAL
#ifdef LTLINUX
#define AL_LIBTYPE_STATIC 1
#include <AL/al.h>
#include <AL/alc.h>
#elif LTMINGW
#define AL_LIBTYPE_STATIC 1
#include <AL/al.h>
#include <AL/alc.h>
#elif LTANDROID
#define AL_LIBTYPE_STATIC 1
#include <AL/al.h>
#include <AL/alc.h>
#elif LTTIZEN
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

// vorbis
extern "C" {
#include "stb_vorbis.h"
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

// Tizen specific
#ifdef LTTIZEN
#include <FBase.h>
#include <FApp.h> 
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
#endif

// Windows specific
#ifdef LTMINGW
#include <windows.h>
#endif

// GLFW
#if defined(LTLINUX) || defined(LTMINGW) || defined(LTOSX)
#define LTGLFW
#include <GLFW/glfw3.h>
#endif

// Curl
#define CURL_STATICLIB 1
#include <curl/curl.h>

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
#include "ltbox2d.h"
#include "ltprotocol.h"
#include "ltrandom.h"
#include "ltresource.h"
#include "ltstate.h"
#include "lttime.h"
#include "lttween.h"
#include "ltaudio.h"
#include "ltvector.h"
#include "ltmesh.h"
#include "ltrendertarget.h"
#include "ltparticles.h"
#include "lttext.h"
#include "ltstore.h"
#include "ltfilestore.h"
#include "ltwavefront.h"
#include "ltlighting.h"
#include "ltjson.h"
#include "lthttp.h"
#include "ltsha1.h"
#include "ltverify.h"
#include "ltluacache.h"

#include "ltgamecenter.h"
#include "ltios.h"
#include "ltiosutil.h"

#include "ltosx.h"
#include "ltosxutil.h"
