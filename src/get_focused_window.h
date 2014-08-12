// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_GET_FOCUSED_WINDOW_H_
#define SRC_GET_FOCUSED_WINDOW_H_

#include <string>

// Return 0 on success, error code otherwise.
int getFocusedWindowInfo(
    std::string *title,
    std::string *filename,
    bool *idle);

#endif  // SRC_GET_FOCUSED_WINDOW_H_
