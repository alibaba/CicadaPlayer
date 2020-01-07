#ifndef EGL_CONTEXT_H
#define EGL_CONTEXT_H

#include "base/config.h"

#if TARGET_PLATFORM == PLATFORM_ANDROID

#include <string>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <dlfcn.h>
#include <jni.h>

#include "gl_context.h"
#include "base/log.h"

namespace Cicada{

    class CicadaEGLContext : public GLContext {
    public:
        CicadaEGLContext() = default;

        virtual ~CicadaEGLContext() = default;

        int Init(void *sharedContext) override;

        void Destroy() override;

        void MakeCurrent(GLSurface *surface) override;

        int Present(GLSurface const *surface) override;

        void PresentationTime(const GLSurface *surface, int64_t pts) override;

        bool SetView(void *view) override;

        void DestroyView() override;

        bool IsViewSizeChanged() override ;

        GLSurface *CreateSurface() override;

        void DestroySurface(GLSurface *&surface) override;

        int GetVisualFormat() override;

        int GetWidth() override;

        int GetHeight() override;

        void *GetContext() override;

        int GetGLVersion() override;

    private:
        EGLConfig chooseBestConfig(int version);

    private:
        EGLContext mContext{EGL_NO_CONTEXT};
        EGLDisplay mDisplay{EGL_NO_DISPLAY};
        EGLConfig mConfig{nullptr};
        EGLSurface mNoSurface{EGL_NO_SURFACE};
        std::string mExtensions;
        EGLint mClientVersion{2};
        int mWidth{0};
        int mHeight{0};
        EGLint mMajor{0};
        EGLint mMinor{0};

        void *mView = nullptr;
        jobject mSurfaceRef = nullptr;


    };

/*****************************************************************************
 * Ndk symbols
 *****************************************************************************/
    typedef khronos_stime_nanoseconds_t EGLnsecsANDROID;

    typedef EGLBoolean (*CicadaEglPresentationTimeANDROID)(EGLDisplay dpy, EGLSurface sur,
                                                          EGLnsecsANDROID time);

    struct syms {
        struct {
            CicadaEglPresentationTimeANDROID cicadaEglPresentationTimeANDROID;
        } CicadaEgl;
    };
    extern struct syms egl_syms;
    struct members {
        const char *name;
        unsigned long offset;
        bool critical;
    };

#define OFFMC(x) (unsigned long)(&(((struct syms *)0)->CicadaEgl.x))
    static struct members eglFmembers[] =
            {
                    {"eglPresentationTimeANDROID", OFFMC(cicadaEglPresentationTimeANDROID), true},
                    {NULL, 0,                                                              false}
            };


/* Initialize all symbols.
 * Done only one time during the first initialisation */
    static bool InitSymbols() {
        static int initState = -1;
        bool ret;
        if (egl_syms.CicadaEgl.cicadaEglPresentationTimeANDROID) {
            RLOGI("Egl .so already loaded");
            return true;
        }
        if (initState != -1) {
            ret = (initState == 1);
            if (!ret) {
                RLOGE("EGL init failed");
            }
            return ret;
        }
        initState = 0;
        void *ndk_handle = dlopen("libEGL.so", RTLD_NOW);
        if (!ndk_handle) {
            goto end;
        }
        for (int i = 0; eglFmembers[i].name; i++) {
            RLOGI("function name %s", eglFmembers[i].name);
            void *sym = dlsym(ndk_handle, eglFmembers[i].name);
            RLOGI("sym %p, critical %s", sym, eglFmembers[i].critical ? "true" : "false");
            if (!sym && eglFmembers[i].critical) {
                dlclose(ndk_handle);
                goto end;
            }
            *(void **) ((uint8_t *) &egl_syms + eglFmembers[i].offset) = sym;
        }

        initState = 1;
        end:
        ret = (initState == 1);
        if (!ret) {
            RLOGE("EGL init failed");
        }
        return ret;
    }


} // namespace Cicada


#endif

#endif // EGL_CONTEXT_H
