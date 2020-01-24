// Copyright 2014 Toggl Desktop developers.

#include "../src/feedback.h"

#include <sstream>

#include "./formatter.h"
#include "./https_client.h"

#include "Poco/Path.h"

namespace toggl {

const std::string Feedback::filename() const {
    Poco::Path p(true);
    bool ok = p.tryParse(attachment_path_);
    if (!ok) {
        return "";
    }
    return p.getFileName();
}

toggl::error Feedback::Validate() const {
    if (subject_.empty()) {
        return toggl::error("Missing topic");
    }
    if (details_.empty()) {
        return toggl::error("Missing details");
    }
    return toggl::noError;
}

}  // namespace toggl
