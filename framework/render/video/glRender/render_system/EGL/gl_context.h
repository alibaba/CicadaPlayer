#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H
#include <stdint.h>
#include <mutex>

namespace Cicada
{

struct GLSurface
{
    void*        surface{nullptr};
    void*        nativeWindow{nullptr};
    unsigned int framebuffer{0};
    unsigned int renderbuffer{0};
};

#define kOpenGLES3_0  3
#define kOpenGLES2_0  2
#define TAG_CONTEXT  "GLContext_TAG"

class GLContext
{
public:
    GLContext() {}
    virtual ~GLContext() {}
    virtual int  Init(void* sharedContext) = 0;
    virtual void Destroy() = 0;
    virtual void MakeCurrent(GLSurface* surface) = 0;
    virtual int  Present(const GLSurface* surface) = 0;
    virtual void PresentationTime(const GLSurface* surface, int64_t pts) = 0;

    /**
     *
     * @param view
     * @return  view changed return true.
     */
    virtual bool SetView(void *view) = 0;
    virtual void DestroyView() = 0;
    virtual bool IsViewSizeChanged() = 0;
    virtual GLSurface *CreateSurface() = 0;
    virtual void DestroySurface(GLSurface*& surface) = 0;
    virtual int  GetVisualFormat() = 0;
    virtual int  GetWidth() = 0;
    virtual int  GetHeight() = 0;
    virtual void* GetContext() = 0;
    virtual int  GetGLVersion() = 0;

    virtual std::mutex& GetMutex(GLSurface* surface)
    {
        static std::mutex mutex;
        return mutex;
    }

    virtual bool IsValid(GLSurface* surface) {return true;}

}; // class GLContext

} // namespace Cicada
#endif // GL_CONTEXT_H
