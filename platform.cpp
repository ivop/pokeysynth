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
    // windows code goes here, reparent HWND
    #error "WIN32 support not implemented yet"
#elif __APPLE__
    // macOS code goes here, reparent NSView
    #error "macOS support not implemented yet"
#else
    #error "Unsupported platform"
#endif
