#include "gl_context_factory.h"
#include <platform/platform_gl.h>
#if TARGET_PLATFORM == PLATFORM_IOS
    #include "platform/ios/eagl_context.h"
#elif TARGET_PLATFORM == PLATFORM_MAC
    #include "platform/mac/mgl_context.h"
#elif TARGET_PLATFORM == PLATFORM_WIN32
    #include "platform/window/wgl_context.h"
#else
    #include "egl_context.h"
#endif

namespace Cicada {
    GLContext *GLContextFactory::NewInstance()
    {
#if TARGET_PLATFORM == PLATFORM_IOS
        return new CicadaEAGLContext();
#elif TARGET_PLATFORM == PLATFORM_ANDROID
        return new CicadaEGLContext();
#elif TARGET_PLATFORM == PLATFORM_MAC
        return new CicadaMGLContext();
#elif TARGET_PLATFORM == PLATFORM_WIN32
        return new CicadaWGLContext();
#else
        return nullptr;
#endif
    }

} // namespace Cicada

