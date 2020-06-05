//
// Created by moqi on 2020/6/4.
//

#ifndef CICADAMEDIA_NATIVEWINDOW_H
#define CICADAMEDIA_NATIVEWINDOW_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

typedef struct {
    const char *tag;
    void *(*CreateNativeWindow)(int w, int h);
    void (*DestroyNativeWindow)(void *window);
} NativeWindowFactory;

#ifdef SDL_VIDEO_DRIVER_WINDOWS
#define TEST_NATIVE_WINDOWS
extern NativeWindowFactory WindowsWindowFactory;
#endif

#ifdef SDL_VIDEO_DRIVER_X11
#define TEST_NATIVE_X11
extern NativeWindowFactory X11WindowFactory;
#endif

#ifdef SDL_VIDEO_DRIVER_COCOA
/* Actually, we don't really do this, since it involves adding Objective C
   support to the build system, which is a little tricky.  You can uncomment
   it manually though and link testnativecocoa.m into the test application.
*/
#define TEST_NATIVE_COCOA
extern NativeWindowFactory CocoaWindowFactory;
#endif

static NativeWindowFactory *getNativeFactor()
{
#ifdef SDL_VIDEO_DRIVER_COCOA
    return &CocoaWindowFactory;
#endif

#ifdef SDL_VIDEO_DRIVER_WINDOWS
    return &WindowsWindowFactory;
#endif
#ifdef SDL_VIDEO_DRIVER_X11
    return &X11WindowFactory;
#endif
    return NULL;
}


#endif//CICADAMEDIA_NATIVEWINDOW_H
