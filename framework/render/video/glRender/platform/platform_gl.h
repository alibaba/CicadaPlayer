#ifndef PLATFORM_GL_H
#define PLATFORM_GL_H
#include "platform_config.h"
#include <utils/frame_work_log.h>

#if TARGET_PLATFORM == PLATFORM_IOS
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#define GL_ES
#elif TARGET_PLATFORM == PLATFORM_ANDROID
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "android/gl3stub.h"
#define GL_ES
#elif TARGET_PLATFORM == PLATFORM_MAC
    #include <OpenGL/gl.h>
    #include <OpenGL/glext.h>
#undef GL_ES
#elif TARGET_PLATFORM == PLATFORM_WIN32
#include <windows.h>
#include <gl/glew.h>
#include <gl/wglew.h>
#undef GL_ES
#endif

#define GL_TEXTURE_EXTERNAL_OES 0x8D65
namespace cicada
{

#if defined(NDEBUG)
#define CHECK_GL_ERROR_DEBUG()
#else
#define CHECK_GL_ERROR_DEBUG() \
    do { \
        GLenum __error = glGetError(); \
        if(__error) { \
            AF_LOGE("OpenGL error 0x%04X in %s %s %d\n", __error, __FILE__, __FUNCTION__, __LINE__); \
        } \
    } while (false)
#endif


} // namespace cicada
#endif // PLATFORM_GL_H
