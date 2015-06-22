// Copyright 2015 Toggl Desktop developers

#include "../src/urls.h"

namespace toggl {

namespace urls {

bool use_staging_as_backend = false;

std::string API() {
    if (use_staging_as_backend) {
        return "https://next.toggl.com";
    }
    return "https://www.toggl.com";
}

std::string TimelineUpload() {
    if (use_staging_as_backend) {
        return "https://next.toggl.com";
    }
    return "https://timeline.toggl.com";
}

std::string WebSocket() {
    if (use_staging_as_backend) {
        return "https://fubar-ws.toggl.com";
    }
    return "https://stream.toggl.com";
}

void SetUseStagingAsBackend(const bool value) {
    use_staging_as_backend = value;
}

}  // namespace urls

}  // namespace toggl
