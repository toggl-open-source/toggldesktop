// Copyright 2014 Toggl Desktop developers.

#include <Carbon/Carbon.h>
#include <CoreGraphics/CGWindow.h>

#include <string>

#include "get_focused_window.h"

#if defined(__APPLE__)
int macOSGetActiveWindowInfo(std::string *title, std::string *filename);
#endif

int getFocusedWindowInfo(
    std::string *title,
    std::string *filename,
    bool *idle) {
    *idle = false;
    return macOSGetActiveWindowInfo(title, filename);
}
