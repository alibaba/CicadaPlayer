#include <render/video/glRender/render_system/EGL/gl_context.h>
#include "render_system/EGL/gl_context.h"
#include "platform/platform_config.h"

#if TARGET_PLATFORM == PLATFORM_MAC
#ifdef __OBJC__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <AppKit/AppKit.h>
#else
struct NSOpenGLContext;
struct NSOpenGLView;
#endif

namespace Cicada
{

class CicadaMGLContext final : public GLContext
{
public:
    CicadaMGLContext() = default;
    ~CicadaMGLContext() = default;
    int  Init(void* sharedContext) override;
    void Destroy() override;
    void MakeCurrent(GLSurface* surface) override;
    int  Present(const GLSurface* surface) override;
    void PresentationTime(const GLSurface* surface, int64_t pts) override ;

    GLSurface *CreateSurface() override;
    void DestroySurface(GLSurface* &surface) override;


    /**
     *
     * @param view
     * @return  view changed return true.
     */
     bool SetView(void *view) override ;
     void DestroyView() override;
     bool IsViewSizeChanged() override;

    int   GetVisualFormat() override;
    void* GetContext() override;
    int   GetWidth() override;
    int   GetHeight() override;
    int   GetGLVersion() override;
    std::mutex& GetMutex(GLSurface* surface) override;
    bool  IsValid(GLSurface* surface) override;

private:
    NSOpenGLContext*  mContext{nullptr};
    NSOpenGLView*     mSetView{nullptr};
    bool              mViewInited = false;
    GLSurface*        mCurrentSurface{nullptr};
    int               mWidth{0};
    int               mHeight{0};
    std::mutex        mViewMutex;
//    std::mutex        mCreateMutex;
};
} // namespace cicada
#endif

