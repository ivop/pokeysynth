#pragma once
// ****************************************************************************
//
// This file is part of PokeySynth.
//
// Copyright © 2024, 2025, by Ivo van Poorten
//
// Licensed under the terms of the General Public License, version 2.
// See the LICENSE file in the root of the prohect directory for the full text.
//
// ****************************************************************************

#include "fltk.h"

#if __linux__ || __APPLE__
#define PATH_SEPARATOR '/'
#elif _WIN32
#define PATH_SEPARATOR '\\'
#endif

void ReparentWindow(Window w, Window parentWindow);
