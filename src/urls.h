// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_URLS_H_
#define SRC_URLS_H_

#include <string>

#include "./const.h"

namespace toggl {

namespace urls {

std::string API();
std::string TimelineUpload();
std::string WebSocket();

}  // namespace urls

}  // namespace toggl

#endif  // SRC_URLS_H_
