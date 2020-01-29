// Copyright 2014 Toggl Desktop developers.

#include "feedback.h"

#include "net/https_client.h"
#include "util/formatter.h"

#include <sstream>

#include <Poco/Path.h>

namespace toggl {

const std::string Feedback::filename() const {
    Poco::Path p(true);
    bool ok = p.tryParse(attachment_path_);
    if (!ok) {
        return "";
    }
    return p.getFileName();
}

error Feedback::Validate() const {
    if (subject_.empty()) {
        return error::kMissingFeedbackTopic;
    }
    if (details_.empty()) {
        return error::kMissingFeedbackDetails;
    }
    return toggl::noError;
}

}  // namespace toggl
