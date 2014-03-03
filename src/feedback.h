// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_FEEDBACK_H_
#define SRC_FEEDBACK_H_

#include "./feedback.h"

#include <string>

#include "./types.h"

namespace kopsik {

class Feedback {
  public:
    Feedback(
      const std::string topic,
      const std::string details,
      const std::string attachment_path)
      : subject_(topic)
      , details_(details)
      , attachment_path_(attachment_path)
      , app_version_("") {}
    ~Feedback() {}

    kopsik::error Validate() const;
    const std::string JSON() const;
    void SetAppVersion(const std::string value) { app_version_ = value; }
  private:
    const std::string filename() const;
    const std::string base64encode_attachment() const;

    std::string subject_;
    std::string details_;
    std::string attachment_path_;
    std::string app_version_;
};

}  // namespace kopsik

#endif  // SRC_FEEDBACK_H_
