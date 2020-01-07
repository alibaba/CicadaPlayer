#ifndef EAGL_CONTEXT_H
#define EAGL_CONTEXT_H
#include "render_system/EGL/gl_context.h"
#include "platform/platform_config.h"
#import "../../render_system/EGL/gl_context.h"

#if TARGET_PLATFORM == PLATFORM_IOS
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

#ifdef __OBJC__
#import <OpenGLES/EAGL.h>
#import <QuartzCore/CAEAGLLayer.h>
#else
struct CAEAGLLayer;
struct EAGLContext;
#endif

#ifdef __OBJC__
#define CICADA_UNRETAINED  __unsafe_unretained
#define CICADA_STRONG      __strong
#else
#define CICADA_UNRETAINED
#define CICADA_STRONG
#endif
namespace Cicada
{

class CicadaEAGLContext final : public GLContext
{
public:
    CicadaEAGLContext() = default;
    ~CicadaEAGLContext() = default;
    int  Init(void* sharedContext) override;
    void Destroy() override;
    void MakeCurrent(GLSurface* surface) override;
    int  Present(const GLSurface* surface) override;
    void PresentationTime(const GLSurface* surface, int64_t pts) override ;

    bool SetView(void *view) override;
    void DestroyView() override;
    bool IsViewSizeChanged() override ;

    GLSurface *CreateSurface() override;
    void DestroySurface(GLSurface* &surface) override;

    int   GetVisualFormat() override;
    int   GetWidth() override;
    int   GetHeight() override;
    void* GetContext() override;
    int   GetGLVersion() override;
private:
    EAGLContext* CICADA_STRONG mContext{nullptr};
    GLSurface*                mCurrentSurface{nullptr};
    int                       mGLVersion{kOpenGLES3_0};
    int         mWidth{0};
    int         mHeight{0};
    
    int mLayerWidth{0};
    int mLayerHeight{0};

    void * mView = nullptr;
};
} // namespace cicada

#endif // EAGL_CONTEXT_H
