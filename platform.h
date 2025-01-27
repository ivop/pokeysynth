#pragma once
#include "fltk.h"

#if __linux__ || __APPLE__
#define PATH_SEPARATOR '/'
#elif _WIN32
#define PATH_SEPARATOR '\\'
#endif

void ReparentWindow(Window w, Window parentWindow);
