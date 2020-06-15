//
// Created by kongjian.kongjian on 2020/06/15.
//
#include "nativeWindow.h"

#ifdef TEST_NATIVE_WINDOWS

#include <winuser.h>

static void *CreateWindowWindows(int w, int h);
static void DestroyWindowWindows(void *window);

NativeWindowFactory WindowsWindowFactory = {
    "windows",
    CreateWindowWindows,
    DestroyWindowWindows
};

static void *
CreateWindowWindows(int w, int h)
{
    WNDCLASSEX wcx;

    // Fill in the window class structure with parameters
    // that describe the main window.

    wcx.cbSize = sizeof(wcx);           // size of structure
    wcx.style = CS_HREDRAW | CS_VREDRAW;// redraw if size changes
    wcx.lpfnWndProc = DefWindowProc;      // points to window procedure
    wcx.cbClsExtra = 0;                 // no extra class memory
    wcx.cbWndExtra = 0;                 // no extra window memory
    wcx.hInstance = GetModuleHandle(NULL);          // handle to instance
    wcx.hIcon = LoadIcon(NULL,
                         IDI_APPLICATION);// predefined app. icon
    wcx.hCursor = LoadCursor(NULL,
                             IDC_ARROW);            // predefined arrow
    wcx.hbrBackground = GetStockObject(WHITE_BRUSH);// white background brush
    wcx.lpszMenuName = "MainMenu";                  // name of menu resource
    wcx.lpszClassName = "MainWClass";               // name of window class
    wcx.hIconSm = LoadImage(GetModuleHandle(NULL),              // small class icon
                            MAKEINTRESOURCE(5), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    // Register the window class.
    RegisterClassEx(&wcx); 

    HWND hwnd;
    // Create the main window.
    hwnd = CreateWindow("MainWClass",       // name of window class
                        "TestPlayer",           // title-bar string
                        WS_OVERLAPPEDWINDOW,// top-level window
                        CW_USEDEFAULT,      // default horizontal position
                        CW_USEDEFAULT,      // default vertical position
                        w,      // default width
                        h,      // default height
                        (HWND) NULL,        // no owner window
                        (HMENU) NULL,       // use class menu
                        GetModuleHandle(NULL),          // handle to application instance
                        (LPVOID) NULL);     // no window-creation data 
    ShowWindow(hwnd, SW_SHOW);
    return (void *)hwnd;
}

static void
DestroyWindowWindows(void *window)
{
    DestroyWindow(window);
}

#endif
