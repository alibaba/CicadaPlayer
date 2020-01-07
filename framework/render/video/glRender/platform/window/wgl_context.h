#ifndef WGL_CONTEXT_H
#define WGL_CONTEXT_H
#include "platform/platform_config.h"
#if TARGET_PLATFORM == PLATFORM_WIN32
#include <string>
#include "render_system/EGL/gl_context.h"
#include <windows.h>
#include <Wingdi.h>
#include <render/glRender/render_system/EGL/gl_context.h>

namespace cicada
{
struct CicadaSurface : public GLSurface
{
    HDC hDC{ nullptr };
    HGLRC hglrc{ nullptr };
};

class CicadaWGLContext : public GLContext
{
public:
    CicadaWGLContext() = default;
    virtual ~CicadaWGLContext() = default;
    int Init(void* sharedContext) override;
    void Destroy() override;

    void MakeCurrent(GLSurface* surface) override;
    int  Present(GLSurface const* surface) override;
    void PresentationTime(const GLSurface* surface, int64_t pts) override ;

    cicada::GLSurface *CreateSurface() override;
    void  DestroySurface(GLSurface* &surface) override;
    int   GetVisualFormat() override;
    int  GetWidth() override;
    int  GetHeight()override;
    void* GetContext() override;
    int  GetGLVersion()override;

private:
    HDC      mHDC{ nullptr };
    HGLRC    mHRC{ nullptr };
    HWND     mHWnd{ nullptr };
    int      mWidth{ 0 };
    int      mHeight{ 0 };
};
} // namespace cicada
#endif // win32
#endif // WGL_CONTEXT_H
