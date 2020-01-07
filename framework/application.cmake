set(ANDROID_TOOLCHAIN gcc)
set(ANDROID_PLATFORM android-18)
set(NDK_APP_STL gnustl_static)

include($ENV{NDK_ROOT}/build/cmake/android.toolchain.cmake)