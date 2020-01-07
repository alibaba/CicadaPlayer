#include "wgl_context.h"
#if TARGET_PLATFORM == PLATFORM_WIN32
#include "base/log.h"
#include <gl/glew.h>
#include <gl/wglew.h>

namespace Cicada {
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg) {
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    int CicadaWGLContext::Init(void *sharedContext)
    {
        try {
            HMODULE hinst = GetModuleHandle(NULL);
            const wchar_t wnd_class_name[] = L"WGL Context Window";
            WNDCLASS wc = {};
            wc.lpfnWndProc = WindowProc;
            wc.hInstance = hinst;
            wc.lpszClassName = wnd_class_name;
            RegisterClass(&wc);
            mHWnd = CreateWindowEx(
                        0,
                        wnd_class_name,
                        L"WGL Context",
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                        NULL,
                        NULL,
                        hinst,
                        NULL
                    );

            if (mHWnd == NULL) {
                RLOGE("Failed to create context window \n");
                return -1;
            }

            mHDC = ::GetDC(mHWnd);

            if (nullptr == mHDC) {
                return -1;
            }

            static PIXELFORMATDESCRIPTOR pfd = {
                sizeof(PIXELFORMATDESCRIPTOR),    //上述格式描述符的大小
                1,                                //版本号
                PFD_DRAW_TO_WINDOW |              //格式支持窗口
                PFD_SUPPORT_OPENGL |              //格式必须支持OpenGL
                PFD_DOUBLEBUFFER,                 //必须支持双缓冲
                PFD_TYPE_RGBA,                    //申请RGBA 格式
                24,                               //选定色彩深度
                0, 0, 0, 0, 0, 0, 0, 0,           //忽略RGBA
                0,                                //无累加缓存
                0, 0, 0, 0,                       //忽略聚集位
                32,                               //32位Z-缓存(深度缓存)
                0,                                //无蒙板缓存
                0,                                //无辅助缓存
                PFD_MAIN_PLANE,                   //主绘图层
                0,                                //Reserved
                0, 0, 0                           //忽略层遮罩
            };
            int nPixelFormat = ChoosePixelFormat(mHDC, &pfd);

            if (0 == nPixelFormat) {
                RLOGE("ChoosePixelFormat fail\n");
                return -1;
            }

            if (::SetPixelFormat(mHDC, nPixelFormat, &pfd)) {
                mHRC = ::wglCreateContext(mHDC);

                if (::wglMakeCurrent(mHDC, mHRC)) {
                    glewExperimental = true;
                    GLenum retVal = glewInit();

                    if (retVal == GLEW_OK) {
                        RLOGI("Init glew sucess\n");
                    }

                    if (GLEW_VERSION_3_2) {
                        RLOGI("OpengGL version 3.2 is load ok");
                    } else {
                        RLOGE("OpengGL version 3.2 load init failed");
                    }

                    if (nullptr == glBindBuffer) {
                        return -1;
                    }

                    return retVal == GLEW_OK ? 0 : -1;
                } else {
                    RLOGE("Failed to set pixformate\n");
                    wglDeleteContext(mHRC);
                    DestroyWindow(mHWnd);
                    mHWnd = nullptr;
                }
            } else {
                RLOGE("Failed to make context current\n");
                DestroyWindow(mHWnd);
                mHWnd = nullptr;
            }
        } catch (...) {
            RLOGE("get exception\n");
        }

        return -1;
    }

    int CicadaWGLContext::Present(const GLSurface *surface)
    {
        if (surface == nullptr || surface->window == nullptr) {
            glFlush();
        } else {
            CicadaSurface *wgl_surface = (CicadaSurface *)surface;

            if (wgl_surface->hDC != nullptr) {
                SwapBuffers(wgl_surface->hDC);
            }
        }

        return 0;
    }

    void CicadaWGLContext::PresentationTime(const GLSurface *surface, int64_t pts)
    {
    }

    void CicadaWGLContext::MakeCurrent(GLSurface *surface)
    {
        RLOGI("MakeCurrent, surface: %p\n", surface);

        if (surface == nullptr || surface->window == nullptr) {
            mWidth = 0;
            mHeight = 0;
            return;
        }

        RLOGI("MakeCurrent, window: %p\n", surface->window);
        CicadaSurface *wgl_surface = (CicadaSurface *)surface;
        RLOGI("MakeCurrent, hdc: %p, context: %p\n", wgl_surface->hDC, wgl_surface->hglrc);

        if (FALSE == wglMakeCurrent(wgl_surface->hDC, wgl_surface->hglrc)) {
            RLOGE("Failed to make current context\n");
        }

        RECT rect;
        GetClientRect((HWND)surface->window, &rect);
        mHeight = rect.bottom - rect.top;
        mWidth = rect.right - rect.left;
    }

    void CicadaWGLContext::Destroy()
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(mHRC);
        ::ReleaseDC(mHWnd, mHDC);
        DestroyWindow(mHWnd);
    }

    GLSurface *CicadaWGLContext::CreateSurface(void *window, int width, int height)
    {
        if (window == nullptr || width < 0 || height < 0) {
            return NULL;
        }

        RLOGI("CreateSurface, window: %p", window);
        CicadaSurface *surface = new CicadaSurface();
        surface->window = window;
        HDC hDC = GetDC((HWND)window);

        if (nullptr == hDC) {
            return nullptr;
        }

        static PIXELFORMATDESCRIPTOR pfd = {
            sizeof(PIXELFORMATDESCRIPTOR),    //上述格式描述符的大小
            1,                                //版本号
            PFD_DRAW_TO_WINDOW |              //格式支持窗口
            PFD_SUPPORT_OPENGL |              //格式必须支持OpenGL
            PFD_DOUBLEBUFFER,                 //必须支持双缓冲
            PFD_TYPE_RGBA,                    //申请RGBA 格式
            24,                               //选定色彩深度
            0, 0, 0, 0, 0, 0, 0, 0,           //忽略RGBA
            0,                                //无累加缓存
            0, 0, 0, 0,                       //忽略聚集位
            32,                               //32位Z-缓存(深度缓存)
            0,                                //无蒙板缓存
            0,                                //无辅助缓存
            PFD_MAIN_PLANE,                   //主绘图层
            0,                                //Reserved
            0, 0, 0                           //忽略层遮罩
        };
        int nPixelFormat = ChoosePixelFormat(hDC, &pfd);
        SetPixelFormat(hDC, nPixelFormat, &pfd);
        wglMakeCurrent(hDC, mHRC);
        surface->hglrc = mHRC;
        surface->hDC = hDC;
        RLOGI("CreateSurface, context: %p, hdc:%p \n", surface->hglrc, surface->hDC);
        return surface;
    }

    void CicadaWGLContext::DestroySurface(GLSurface *&surface)
    {
        if (surface == nullptr) {
            return;
        }

        RLOGI("DestroySurface, surface: %p, window: %p \n", surface, surface->window);
        CicadaSurface *wgl_surface = static_cast<CicadaSurface *>(surface);
        RLOGI("DestroySurface, context: %p, hdc: %p \n", wgl_surface->hglrc, wgl_surface->hDC);
        wglMakeCurrent(nullptr, nullptr);
        //wglDeleteContext(wgl_surface->hglrc);
        ::ReleaseDC((HWND)wgl_surface->window, wgl_surface->hDC);
        delete wgl_surface;
        surface = nullptr;
    }

    int CicadaWGLContext::GetVisualFormat()
    {
        return 0;
    }


    int CicadaWGLContext::GetWidth()
    {
        return mWidth;
    }


    int CicadaWGLContext::GetHeight()
    {
        return mHeight;
    }

    void *CicadaWGLContext::GetContext()
    {
        return NULL;
    }


    int CicadaWGLContext::GetGLVersion()
    {
        return 2;
    }

} // namespace cicada
#endif // win32
