// Copyright 2014 Toggl Desktop developers.

#include "./feedback.h"

#include <sstream>

#include "./formatter.h"

#include "Poco/FileStream.h"
#include "Poco/Base64Encoder.h"
#include "Poco/StreamCopier.h"
#include "Poco/Path.h"

namespace kopsik {

const std::string Feedback::JSON() const {
  JSONNODE *root = json_new(JSON_NODE);
  json_push_back(root, json_new_b("desktop", true));
  json_push_back(root, json_new_a("toggl_version", app_version_.c_str()));
  json_push_back(root, json_new_a("details",
    Formatter::EscapeJSONString(details_).c_str()));
  json_push_back(root, json_new_a("subject",
    Formatter::EscapeJSONString(subject_).c_str()));
  if (!attachment_path_.empty()) {
    json_push_back(root, json_new_a("base64_encoded_attachment",
      base64encode_attachment().c_str()));
    json_push_back(root, json_new_a("attachment_name",
      Formatter::EscapeJSONString(filename()).c_str()));
  }
  json_char *jc = json_write_formatted(root);
  std::string json(jc);
  json_free(jc);
  json_delete(root);
  return json;
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

kopsik::error Feedback::Validate() const {
  if (subject_.empty()) {
    return kopsik::error("Missing topic");
  }
  if (details_.empty()) {
    return kopsik::error("Missing details");
  }
  if (app_version_.empty()) {
    return kopsik::error("Missing app version");
  }
  return kopsik::noError;
}

}  // namespace kopsik
