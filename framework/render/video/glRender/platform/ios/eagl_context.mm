#include "eagl_context.h"

#if TARGET_PLATFORM == PLATFORM_IOS

#include "../../base/error_event_def.h"
#include "base/log.h"
#include "platform/platform_gl.h"

namespace Cicada{

    int CicadaEAGLContext::Init(void *sharedContext) {
        if (sharedContext == nullptr) {
            mContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        } else {
            mContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3
                                             sharegroup:((__bridge EAGLContext *) sharedContext).sharegroup];
        }
        if (!mContext) {
            if (sharedContext == nullptr) {
                mContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
            } else {
                mContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2
                                                 sharegroup:((__bridge EAGLContext *) sharedContext).sharegroup];
            }
            mGLVersion = kOpenGLES2_0;
        }
        if (mContext == nil) {
            RLOGE("EAGLContext init failed version %d", mGLVersion);
            return CICADA_FRAMEWORK_RENDER_ERROR_GL_CONTEXT_INIT_FAILED;
        }
        [EAGLContext setCurrentContext:mContext];
        return CICADA_COMMON_RETURN_SUCCESS;
    }

    bool CicadaEAGLContext::SetView(void *view) {
        if(mView == view) {
            return false;
        }else{
            mView = view;
            if (mCurrentSurface != nullptr) {
                mCurrentSurface->surface = mView;
            }
            return true;
        }
    }

    void CicadaEAGLContext::DestroyView() {

    }

    GLSurface *CicadaEAGLContext::CreateSurface() {
        if (mView == nullptr) {
            RLOGE("EAGLContext create surface failed by window is nil");
            return nullptr;
        }
        GLint fbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
        GLSurface *surface = new GLSurface();
        surface->surface = mView;
        glGenFramebuffers(1, &surface->framebuffer);
        CHECK_GL_ERROR_DEBUG();
        glBindFramebuffer(GL_FRAMEBUFFER, surface->framebuffer);
        CHECK_GL_ERROR_DEBUG();
        glGenRenderbuffers(1, &surface->renderbuffer);
        CHECK_GL_ERROR_DEBUG();
        glBindRenderbuffer(GL_RENDERBUFFER, surface->renderbuffer);

        CHECK_GL_ERROR_DEBUG();
        [mContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:(__bridge CAEAGLLayer *) surface->surface];
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, surface->renderbuffer);
        CHECK_GL_ERROR_DEBUG();

        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &mWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &mHeight);
        RLOGD("surface buffer width %d height %d", mWidth, mHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        CHECK_GL_ERROR_DEBUG();

        CAEAGLLayer *layer = (__bridge CAEAGLLayer *) surface->surface;
        mLayerWidth = layer.frame.size.width;
        mLayerHeight = layer.frame.size.height;

        return surface;
    }

    void CicadaEAGLContext::DestroySurface(GLSurface *&surface) {
        if (surface == nullptr) {
            return;
        }
        if ([EAGLContext currentContext] != mContext) {
            [EAGLContext setCurrentContext:mContext];
        }
        auto renderbuffer = surface->renderbuffer;
        auto framebuffer = surface->framebuffer;

        if (framebuffer) {
            if (glIsFramebuffer(framebuffer)) {
                glDeleteFramebuffers(1, &framebuffer);
            } else {
                RLOGW("fbo(%d) not delete, maybe leak", framebuffer);
            }
            framebuffer = 0;
        }

        if (renderbuffer) {
            GLint rbo;
            glGetIntegerv(GL_RENDERBUFFER_BINDING, &rbo);
            CHECK_GL_ERROR_DEBUG();
            if (glIsRenderbuffer(renderbuffer)) {
                glDeleteRenderbuffers(1, &renderbuffer);
            } else {
                RLOGW("render buffer(%d) not delete, maybe leak", renderbuffer);
            }
            CHECK_GL_ERROR_DEBUG();
            renderbuffer = 0;
        }
        delete surface;
        surface = nullptr;
    }

    void CicadaEAGLContext::MakeCurrent(GLSurface *surface) {
        GLint fbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
        if (!surface) {
//            RLOGW("EAGLContext makeCurrent failed by surface is null");
            mCurrentSurface = surface;
            return;
        }
        if (surface == mCurrentSurface && fbo == surface->framebuffer) {
        //    RLOGW("EAGLContext makeCurrent ignore");
            return;
        }
        if ([EAGLContext currentContext] != mContext) {
            [EAGLContext setCurrentContext:mContext];
        }
        mCurrentSurface = surface;
        CHECK_GL_ERROR_DEBUG();
        if (surface == nullptr || surface->surface == nullptr) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            CHECK_GL_ERROR_DEBUG();
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            CHECK_GL_ERROR_DEBUG();
            RLOGW("EAGLContext makeCurrent failed by surface is null or "
                  "surface->window is null");
            return;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, surface->framebuffer);
        CHECK_GL_ERROR_DEBUG();
        glBindRenderbuffer(GL_RENDERBUFFER, surface->renderbuffer);
        CHECK_GL_ERROR_DEBUG();
        [mContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:(__bridge CAEAGLLayer *) surface->surface];
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, surface->renderbuffer);
        CHECK_GL_ERROR_DEBUG();

        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &mWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &mHeight);
        RLOGD("MakeCurrent surface buffer width %d height %d", mWidth, mHeight);

    }

    int CicadaEAGLContext::Present(const GLSurface *surface) {
        [mContext presentRenderbuffer:GL_RENDERBUFFER];
        return 0;
    }

    void CicadaEAGLContext::PresentationTime(const GLSurface *surface, int64_t pts) {

    }

    void CicadaEAGLContext::Destroy() {
        [EAGLContext setCurrentContext:nil];
        //   [mContext release];
    }

    int CicadaEAGLContext::GetVisualFormat() { return 0; }

    int CicadaEAGLContext::GetGLVersion() { return mGLVersion; }

    int CicadaEAGLContext::GetWidth() { return mWidth; }

    int CicadaEAGLContext::GetHeight() { return mHeight; }

    void *CicadaEAGLContext::GetContext()
    {
        return (__bridge void *) mContext;
    }

    bool CicadaEAGLContext::IsViewSizeChanged(){

        if(mCurrentSurface == nullptr || mCurrentSurface->surface == nullptr){
            return false;
        }

        CAEAGLLayer *layer = (__bridge CAEAGLLayer *) mCurrentSurface->surface;
        if(layer == nullptr){
            return false;
        }

        int layerWidth = layer.frame.size.width;
        int layerHeight = layer.frame.size.height;

        bool changed = (mLayerWidth != layerWidth || mLayerHeight != layerHeight);
        return changed;
    };
} // namespace cicada

#endif
