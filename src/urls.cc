// Copyright 2015 Toggl Desktop developers

#include "../src/urls.h"

namespace toggl {

namespace urls {

// Whether requests are sent to staging backend

bool use_staging_as_backend = false;

void SetUseStagingAsBackend(const bool value) {
    use_staging_as_backend = value;
}

std::string API() {
    if (use_staging_as_backend) {
        return "https://toggl.space";
    }
    return "https://desktop.toggl.com";
}

std::string TimelineUpload() {
    if (use_staging_as_backend) {
        return "https://toggl.space";
    }
    return "https://timeline.toggl.com";
}

std::string WebSocket() {
    if (use_staging_as_backend) {
        return "https://stream.toggl.space";
    }
    return "https://stream.toggl.com";
}

// Whether requests are allowed at all (like in tests)

bool requests_allowed_ = true;

bool RequestsAllowed() {
    return requests_allowed_;
}

void SetRequestsAllowed(const bool value) {
    requests_allowed_ = value;
}

// Whether requests are allowed to Toggl backend

bool im_a_teapot_ = false;

bool ImATeapot() {
    return im_a_teapot_;
}

void SetImATeapot(const bool value) {
    im_a_teapot_ = value;
}

}  // namespace urls

}  // namespace toggl
