#define LOG_TAG "GLRender egl_context"

#include "egl_context.h"

#if TARGET_PLATFORM == PLATFORM_ANDROID

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "base/log.h"
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <utils/frame_work_log.h>

#include <render/video/glRender/base/error_event_def.h>
#include <utils/Android/JniEnv.h>
#include <platform/android/native_window.h>


namespace Cicada {
    struct syms egl_syms;

    int CicadaEGLContext::Init(void *sharedContext)
    {
        bool ret = false;
        ret = InitSymbols();

        if (!ret) {
            AF_LOGE("libEGL.so load failed!");
        }

        if (nullptr == sharedContext) {
            sharedContext = EGL_NO_CONTEXT;
            AF_LOGD("EGLContext sharedContext nullptr");
        }

        mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

        if (nullptr == mDisplay) {
            AF_LOGE("EGLContext get display failed");
            return CICADA_FRAMEWORK_RENDER_ERROR_GL_CONTEXT_INIT_FAILED;
        }

        EGLBoolean rv = eglInitialize(mDisplay, &mMajor, &mMinor);

        if (rv == EGL_FALSE) {
            AF_LOGE("EGLContext eglInitialize failed");
            return CICADA_FRAMEWORK_RENDER_ERROR_GL_CONTEXT_INIT_FAILED;
        }

        AF_LOGI("EGL client major %d minor %d", mMajor, mMinor);
        mExtensions = eglQueryString(mDisplay, EGL_EXTENSIONS);
        AF_LOGI("EGL extension %s", mExtensions.c_str());
        EGLConfig config = chooseBestConfig(2);
        AF_LOGI("EGL choose best config");

        if (nullptr == config) {
            AF_LOGE("not found a suitable config");
            return CICADA_FRAMEWORK_RENDER_ERROR_GL_CONTEXT_INIT_FAILED;
        }

        mConfig = config;
        EGLint context_attrib_list[] = {
            EGL_CONTEXT_CLIENT_VERSION, mClientVersion,
            EGL_NONE,
        };
        AF_LOGI("EGLContext client version %d", mClientVersion);
        mContext = eglCreateContext(mDisplay, config, sharedContext, context_attrib_list);

        if (EGL_NO_CONTEXT == mContext) {
            EGLint err = eglGetError();
            AF_LOGE("eglCreateContext: 0x%X", err);
            return CICADA_FRAMEWORK_RENDER_ERROR_GL_CONTEXT_INIT_FAILED;
        }

        EGLint surface_attrib_list[] {
            EGL_WIDTH, 1,
            EGL_HEIGHT, 1,
            EGL_NONE,
        };
        mNoSurface = eglCreatePbufferSurface(mDisplay, mConfig, surface_attrib_list);

        if (EGL_NO_SURFACE == mNoSurface) {
            EGLint err = eglGetError();
            AF_LOGE("eglCreatePbufferSurface: 0x%X", err);
            return CICADA_FRAMEWORK_RENDER_ERROR_GL_CONTEXT_INIT_FAILED;
        }

        rv = eglMakeCurrent(mDisplay, mNoSurface, mNoSurface, mContext);

        if (EGL_FALSE == rv) {
            EGLint err = eglGetError();
            AF_LOGE("eglMakeCurrent: 0x%x", err);
            return CICADA_FRAMEWORK_RENDER_ERROR_GL_CONTEXT_INIT_FAILED;
        }

        AF_LOGI("eglMakeCurrent: display(%p) surface(%p) context(%p)", mDisplay, mNoSurface,
                mContext);
        return CICADA_COMMON_RETURN_SUCCESS;
    }

    int CicadaEGLContext::Present(const GLSurface *surface)
    {
        EGLBoolean rv = EGL_FALSE;

        if (surface == nullptr || surface->surface == nullptr) {
            glFlush();
            return CICADA_COMMON_RETURN_SUCCESS;
        } else {
            rv = eglSwapBuffers(mDisplay, surface->surface);
//            AF_LOGD("EGLContext eglSwapBuffers client window");
        }

        if (rv != EGL_TRUE) {
            EGLint err = eglGetError();
            AF_LOGE("EGLContext eglSwapBuffers failed: 0x%x", err);
            return CICADA_FRAMEWORK_RENDER_ERROR_GL_SWAP_BUFFER_FAILED;
        }

        return CICADA_COMMON_RETURN_SUCCESS;
    }

    void CicadaEGLContext::PresentationTime(const GLSurface *surface, int64_t pts)
    {
        if (surface) {
            if (egl_syms.CicadaEgl.cicadaEglPresentationTimeANDROID) {
                egl_syms.CicadaEgl.cicadaEglPresentationTimeANDROID(mDisplay, surface->surface,
                        pts * 1000);
            } else {
                AF_LOGE("Egl .so load failed!Cannot use eglPresentationTimeANDROID");
            }
        } else {
            AF_LOGE("Output surface is null, not rendering!");
        }
    }

    void CicadaEGLContext::MakeCurrent(GLSurface *surface)
    {
        EGLBoolean rv;

        if (surface == nullptr || surface->surface == nullptr) {
            rv = eglMakeCurrent(mDisplay, mNoSurface, mNoSurface, mContext);
        } else {
            rv = eglMakeCurrent(mDisplay, surface->surface, surface->surface, mContext);
        }

        if (EGL_TRUE != rv) {
            EGLint err = eglGetError();
            AF_LOGE("renderEngine", "eglMakeCurrent: 0x%x", err);
            return;
        }
    }

    void CicadaEGLContext::Destroy()
    {
        if (EGL_NO_CONTEXT != mContext) {
            eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            EGLBoolean rv = eglDestroySurface(mDisplay, mNoSurface);

            if (EGL_TRUE != rv) {
                EGLint err = eglGetError();
                AF_LOGE("eglDestroySurface: 0x%x", err);
            }

            eglDestroyContext(mDisplay, mContext);

            if (mMajor >= 1 && mMinor >= 2) {
                eglReleaseThread();
            }

            eglTerminate(mDisplay);
        }

        mDisplay = EGL_NO_DISPLAY;
        mContext = EGL_NO_CONTEXT;
        mConfig = nullptr;
    }

    void CicadaEGLContext::DestroyView()
    {
        if (mView != nullptr) {
            delete (NativeWindow *) mView;
            mView = nullptr;
        }
    }

    bool CicadaEGLContext::SetView(void *view)
    {
        //for android, view should be Surface class object
        if (mSurfaceRef == view) {
            return false;
        }

        if (mView != nullptr) {
            delete (Cicada::NativeWindow *) mView;
            mView = nullptr;
        }

        if (view) {
            JniEnv Jenv;
            JNIEnv *mEnv = Jenv.getEnv();
            auto *window = new Cicada::NativeWindow(ANativeWindow_fromSurface(mEnv, (jobject) view));
            mView = window;
        }

        mSurfaceRef = static_cast<jobject>(view);
        return true;
    }

    GLSurface *CicadaEGLContext::CreateSurface()
    {
        if (mView == nullptr) {
//            AF_LOGE("eglCreateWindowSurface failed : window null!");
            return nullptr;
        }

        NativeWindow *nativeWindow = (NativeWindow *) mView;
        AF_LOGI("EGLContext CreateSurface mDisplay %p mConfig %p window %p ", mDisplay, mConfig,
                mView);
        EGLSurface surface = eglCreateWindowSurface(mDisplay, mConfig,
                             (EGLNativeWindowType) (nativeWindow->Get()),
                             nullptr);

        if (EGL_NO_SURFACE == surface) {
            EGLint err = eglGetError();
            AF_LOGE("EGLContext eglCreateWindowSurface failed : 0x%x", err);
            return nullptr;
        }

        AF_LOGI("EGLContext eglCreateWindowSurface surface : %p", surface);
        GLSurface *s = new GLSurface();
        s->surface = surface;
        s->nativeWindow = nativeWindow;
        mWidth = nativeWindow->GetWidth();
        mHeight = nativeWindow->GetHeight();
        return s;
    }

    void CicadaEGLContext::DestroySurface(GLSurface *&surface)
    {
        if (surface == nullptr || surface->surface == nullptr) {
            return;
        }

        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        auto rv = eglDestroySurface(mDisplay, (EGLSurface) surface->surface);
        AF_LOGI("EGLContext eglDestroySurface eglSurface %p", surface->surface);

        if (EGL_TRUE != rv) {
            EGLint err = eglGetError();
            AF_LOGE("eglDestroySurface: 0x%x", err);
        }

        delete surface;
        surface = nullptr;
    }

    int CicadaEGLContext::GetVisualFormat()
    {
        EGLint format = 0;

        if (!eglGetConfigAttrib(mDisplay, mConfig, EGL_NATIVE_VISUAL_ID, &format)) {
            AF_LOGE("eglGetConfigAttrib() returned error %d", eglGetError());
            return 0;
        }

        return format;
    }

    int CicadaEGLContext::GetWidth()
    {
        return mWidth;
    }

    int CicadaEGLContext::GetHeight()
    {
        return mHeight;
    }

    void *CicadaEGLContext::GetContext()
    {
        return mContext;
    }

    int CicadaEGLContext::GetGLVersion()
    {
        return kOpenGLES2_0;
    }


    EGLConfig CicadaEGLContext::chooseBestConfig(int version)
    {
        int attribList[15] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
            EGL_NONE, 0,      // placeholder for recordable [@-3]
            EGL_NONE
        };
        AF_LOGI("EGL attr version %d", version);
        EGLConfig configs[1];
        EGLint num_config;
        EGLBoolean rv = eglChooseConfig(mDisplay, attribList, configs, 1, &num_config);
        AF_LOGI("EGL config num %d", num_config);

        if (EGL_FALSE == rv) {
            return nullptr;
        }

        mConfig = configs[0];
        return configs[0];
    }

    bool CicadaEGLContext::IsViewSizeChanged()
    {
        NativeWindow *nativeWindow = (NativeWindow *) mView;

        if (nativeWindow != nullptr) {
            return ( mWidth != nativeWindow->GetWidth() || mHeight != nativeWindow->GetHeight());
        } else {
            return false;
        }
    }

} // namespace Cicada
#endif
