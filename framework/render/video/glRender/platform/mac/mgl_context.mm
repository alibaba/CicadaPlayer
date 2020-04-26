#include "mgl_context.h"

#if TARGET_PLATFORM == PLATFORM_MAC
#include "CicadaPlayerView.h"

namespace Cicada {
    int CicadaMGLContext::Init(void *sharedContext) {
        NSOpenGLPixelFormatAttribute pixelFormatAttributes[] = {
                NSOpenGLPFADoubleBuffer,
                NSOpenGLPFAAccelerated, 0,
                0
        };

        NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];

        if (sharedContext == nullptr) {
            mContext = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];
        } else {
            mContext = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:(__bridge NSOpenGLContext *) sharedContext];
        }

        [pf release];
        //    GLint major;
        //    GLint minor;
        //    NSOpenGLGetVersion(&major, &minor);
        //    AF_LOGI(TAG_CONTEXT, "video render engine OpenGL version(%d, %d)",
        //    major, minor);
        [mContext makeCurrentContext];
        [mContext clearDrawable];
        return 0;
    }

    GLSurface *CicadaMGLContext::CreateSurface() {
        std::lock_guard<std::mutex> lock(mViewMutex);
        if ((nil == mSetView) || !mViewInited) {
            return nullptr;
        }
        if (nullptr != mCurrentSurface) {
            return mCurrentSurface;
        }
        GLSurface *surface = new GLSurface();
        mCurrentSurface = surface;
        mCurrentSurface->nativeWindow = mSetView;
        mCurrentSurface->surface = mSetView;
        return mCurrentSurface;
    }

    void CicadaMGLContext::DestroySurface(GLSurface *&surface) {
        if (surface == nullptr) {
            return;
        }
        delete surface;
        surface = nullptr;
        mCurrentSurface = nullptr;
    }

    std::mutex &CicadaMGLContext::GetMutex(GLSurface *surface) {
        if (surface) {
            CicadaPlayerView *renderView = (__bridge CicadaPlayerView *) surface->surface;
            return (std::mutex &) [renderView getMutex];
        }

        return GLContext::GetMutex(surface);
    }

    bool CicadaMGLContext::IsValid(GLSurface *surface) {
        if (surface) {
            if (nullptr == surface->nativeWindow) {
                return false;
            }
            CicadaPlayerView *renderView = (__bridge CicadaPlayerView *) surface->nativeWindow;
            return renderView.isValid;
        }

        return false;
    }

    void CicadaMGLContext::MakeCurrent(GLSurface *surface) {
        if (!surface) {
            return;
        }

        CicadaPlayerView *renderView = (__bridge CicadaPlayerView *)surface->nativeWindow;
        [renderView.openGLContext makeCurrentContext];
        mCurrentSurface = surface;
        mContext = [renderView openGLContext];
        NSRect backingBounds = renderView.subBackingBounds;
        mWidth = backingBounds.size.width;
        mHeight = backingBounds.size.height;
    }

    int CicadaMGLContext::Present(const GLSurface *surface) {
        [mContext lock];
        [mContext flushBuffer];
        [mContext unlock];
        return 0;
    }

    void CicadaMGLContext::PresentationTime(const GLSurface *surface, int64_t pts) {

    }

    void CicadaMGLContext::Destroy() {
        [mContext lock];

        [mContext flushBuffer];

        [mContext unlock];

        [NSOpenGLContext clearCurrentContext];

        [mContext release];
        mContext = nil;
    }

    int CicadaMGLContext::GetVisualFormat() { return 0; }

    void *CicadaMGLContext::GetContext() { return mContext; }

    int CicadaMGLContext::GetWidth() { return mWidth; }

    int CicadaMGLContext::GetHeight() { return mHeight; }

    int CicadaMGLContext::GetGLVersion() { return 0; }


    bool CicadaMGLContext::SetView(void *view) {
        std::lock_guard<std::mutex> lock(mViewMutex);
        if (mSetView == view) {
            return false;
        } else {
            mViewInited = false;
            mSetView = (__bridge NSOpenGLView *)view;

            if ([NSThread isMainThread]) {
                [mSetView setOpenGLContext:mContext];
                mViewInited = true;
            } else {
                dispatch_async(dispatch_get_main_queue(), ^{
                    [mSetView setOpenGLContext:mContext];
                    mViewInited = true;
                });
            }

            return true;
        }
    }

    void CicadaMGLContext::DestroyView() {

    }

    bool CicadaMGLContext::IsViewSizeChanged() {
        if (mCurrentSurface == nullptr || mCurrentSurface->surface == nullptr){
            return false;
        }

        CicadaPlayerView *renderView = (__bridge CicadaPlayerView *)mCurrentSurface->surface;
        NSRect backingBounds = renderView.subBackingBounds;
        int width = backingBounds.size.width;
        int height = backingBounds.size.height;
//        bool changed = (mWidth != width || mHeight != height);
        mWidth = width;
        mHeight = height;

        // return false due to don't need recreate surface
        return false;
    }

} // namespace cicada
#endif // mac
