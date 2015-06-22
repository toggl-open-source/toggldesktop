// Copyright 2015 Toggl Desktop developers

#include "../src/urls.h"

namespace toggl {

namespace urls {

std::string API() {
    return "https://www.toggl.com";
}

std::string TimelineUpload() {
    return "https://timeline.toggl.com";
}

std::string WebSocket() {
    return "https://stream.toggl.com";
}

}  // namespace urls

}  // namespace toggl
