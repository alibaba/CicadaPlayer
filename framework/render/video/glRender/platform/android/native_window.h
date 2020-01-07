#ifndef NATIVE_WINDOW_H
#define NATIVE_WINDOW_H
#include <android/native_window.h>
#include <android/native_window_jni.h>
namespace Cicada
{

class NativeWindow
{
public:
    explicit
    NativeWindow(ANativeWindow* window);
    ~NativeWindow();
    ANativeWindow* Get();
    int GetWidth();
    int GetHeight();
private:
    ANativeWindow* mWindow{nullptr};
};

} // namespace cicada
#endif //NATIVE_WINDOW_H
