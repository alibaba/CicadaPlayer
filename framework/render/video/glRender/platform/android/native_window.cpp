#include "native_window.h"
namespace Cicada {
    NativeWindow::NativeWindow(ANativeWindow *window)
    {
        mWindow = window;
    }

    NativeWindow::~NativeWindow()
    {
        if (mWindow != nullptr) {
            ANativeWindow_release(mWindow);
            mWindow = nullptr;
        }
    }

    ANativeWindow *NativeWindow::Get()
    {
        return mWindow;
    }

    int NativeWindow::GetWidth()
    {
        if (mWindow != nullptr) {
            return ANativeWindow_getWidth(mWindow);
        }

        return 0;
    }

    int NativeWindow::GetHeight()
    {
        if (mWindow != nullptr) {
            return ANativeWindow_getHeight(mWindow);
        }

        return 0;
    }

} // namespace cicada
