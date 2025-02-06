#include "platform.h"
#include "fltk.h"

#ifdef __linux__

#include <X11/Xlib.h>
#include <unistd.h>

void ReparentWindow(Window w, Window parentWindow) {
    XSync(fl_display, False);
    Fl::check();
    Fl::flush();
    usleep(100000);

    XUnmapWindow(fl_display, w);

    XSync(fl_display, False);
    Fl::check();
    Fl::flush();
    usleep(100000);

    XReparentWindow(fl_display, w, parentWindow, 0,0);
    XMapWindow(fl_display, w);
    XSync(fl_display, False);
}

#elif _WIN32

unsigned int GetWindowUIState(HWND hwnd) {
    return LOWORD(SendMessage(hwnd, WM_QUERYUISTATE, 0, 0));
}

void ReparentWindow(Window w, Window parentWindow) {
    // Note that Carla still misses around 90% of the keyboard events. This
    // seems to be a bug in Carla, as Reaper works 100% correctly.
    //
    // Change Window Style, setting WS_CHILD blocks all keyboard and mouse
    // events, so we don't set that flag, but we do remove the window's caption
    //
    unsigned int style = GetWindowLong((HWND) w, GWL_STYLE);
    //style = (style | WS_CHILD) & (~WS_POPUP) & (~WS_CAPTION);
    style = (style) & (~WS_POPUP) & (~WS_CAPTION);
    SetWindowLong((HWND) w, GWL_STYLE, style);

    // Reposition at (0,0)
    SetWindowPos((HWND) w, nullptr, 0, 0, 123, 123, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER);
    SetParent((HWND) w, (HWND) parentWindow);

    // Synchronize UI States
    auto parentUIState = GetWindowUIState((HWND) parentWindow);
    auto currentState = GetWindowUIState((HWND) w);
    auto missingState = parentUIState & ~currentState;
    if (missingState) {
        SendMessage((HWND) w, WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, missingState), 0);
    }
    auto extraState = currentState & ~parentUIState;
    if (extraState) {
        SendMessage((HWND) w, WM_UPDATEUISTATE, MAKEWPARAM(UIS_CLEAR, extraState), 0);
    }
}

#elif __APPLE__
    // macOS code goes here, reparent NSView
    #error "macOS support not implemented yet"
#else
    #error "Unsupported platform"
#endif
