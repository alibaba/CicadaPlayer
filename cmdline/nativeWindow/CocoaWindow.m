
#include "nativeWindow.h"

#ifdef TEST_NATIVE_COCOA

#include <Cocoa/Cocoa.h>

static void *CreateWindowCocoa(int w, int h);
static void DestroyWindowCocoa(void *window);

NativeWindowFactory CocoaWindowFactory = {"cocoa", CreateWindowCocoa, DestroyWindowCocoa};

static void *CreateWindowCocoa(int w, int h)
{
    NSAutoreleasePool *pool;
    NSWindow *nswindow;
    NSRect rect;
    unsigned int style;
    NSArray *screens = [NSScreen screens];

    pool = [[NSAutoreleasePool alloc] init];

    rect.origin.x = 0;
    rect.origin.y = 0;
    rect.size.width = w;
    rect.size.height = h;
    rect.origin.y = CGDisplayPixelsHigh(kCGDirectMainDisplay) - rect.origin.y - rect.size.height;

    style = (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable);
    //    NSRect viewport = [contentView bounds];
    //    if ([contentView respondsToSelector:@selector(convertRectToBacking:)]) {
    //        viewport = [contentView convertRectToBacking:viewport];
    //    }
    //    rect.size.width = viewport.size.width;
    //    rect.size.height = viewport.size.height;

    /* Figure out which screen to place this window */
    NSScreen *screen = nil;
    for (NSScreen *candidate in screens) {
        NSRect screenRect = [candidate frame];
        if (rect.origin.x >= screenRect.origin.x && rect.origin.x < screenRect.origin.x + screenRect.size.width &&
            rect.origin.y >= screenRect.origin.y && rect.origin.y < screenRect.origin.y + screenRect.size.height) {
            screen = candidate;
            rect.origin.x -= screenRect.origin.x;
            rect.origin.y -= screenRect.origin.y;
        }
    }

    nswindow = [[NSWindow alloc] initWithContentRect:rect styleMask:style backing:NSBackingStoreBuffered defer:FALSE screen:screen];

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 101200 /* Added in the 10.12.0 SDK. */
    /* By default, don't allow users to make our window tabbed in 10.12 or later */
    if ([nswindow respondsToSelector:@selector(setTabbingMode:)]) {
        [nswindow setTabbingMode:NSWindowTabbingModeDisallowed];
    }
#endif
    [nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    rect = [nswindow contentRectForFrameRect:[nswindow frame]];
    NSView *contentView = [[NSView alloc] initWithFrame:rect];
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
    if ([contentView respondsToSelector:@selector(setWantsBestResolutionOpenGLSurface:)]) {
        //  BOOL highdpi = (window->flags & SDL_WINDOW_ALLOW_HIGHDPI) != 0;
        [contentView setWantsBestResolutionOpenGLSurface:YES];
    }
#ifdef __clang__
#pragma clang diagnostic pop
#endif


    [nswindow makeKeyAndOrderFront:nil];
    [nswindow setContentView:contentView];
    [contentView release];

    [pool release];

    return nswindow;
}

static void DestroyWindowCocoa(void *window)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSWindow *nswindow = (NSWindow *) window;

    [nswindow close];
    [pool release];
}

#endif
