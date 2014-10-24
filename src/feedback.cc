// Copyright 2014 Toggl Desktop developers.

#include "./feedback.h"

#include <sstream>

#include "./formatter.h"
#include "./https_client.h"

#include "Poco/FileStream.h"
#include "Poco/Base64Encoder.h"
#include "Poco/StreamCopier.h"
#include "Poco/Path.h"

namespace toggl {

const std::string Feedback::JSON() const {
    Json::Value root;

    root["desktop"] = true;
    root["toggl_version"] = HTTPSClientConfig::AppVersion;
    root["details"] = Formatter::EscapeJSONString(details_);
    root["subject"] = Formatter::EscapeJSONString(subject_);
    if (!attachment_path_.empty()) {
        root["base64_encoded_attachment"] = base64encode_attachment();
        root["attachment_name"] = Formatter::EscapeJSONString(filename());
    }

    Json::StyledWriter writer;
    return writer.write(root);
}

const std::string Feedback::filename() const {
    Poco::Path p(true);
    bool ok = p.tryParse(attachment_path_);
    if (!ok) {
        return "";
    }
    return p.getFileName();
}

const std::string Feedback::base64encode_attachment() const {
    std::ostringstream oss;
    Poco::FileInputStream fis(attachment_path_);
    if (!fis.good()) {
        return "";
    }
    Poco::Base64Encoder encoder(oss);
    encoder.rdbuf()->setLineLength(0);  // disable line feeds in output
    Poco::StreamCopier::copyStream(fis, encoder);
    encoder.close();
    return oss.str();
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
