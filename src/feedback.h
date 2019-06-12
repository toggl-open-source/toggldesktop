// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_FEEDBACK_H_
#define SRC_FEEDBACK_H_

#include <string>
#include <sstream>  // NOLINT

#include "./types.h"

namespace toggl {

class Feedback {
 public:
    Feedback()
        : subject_("")
    , details_("")
    , attachment_path_("") {}
    ~Feedback() {}

    toggl::error Validate() const;

    const std::string &AttachmentPath() const {
        return attachment_path_;
    }
    void SetAttachmentPath(const std::string &value) {
        attachment_path_ = value;
    }

    const std::string &Details() const {
        return details_;
    }
    void SetDetails(const std::string &value) {
        details_ = value;
    }

    void SetSubject(const std::string &value) {
        std::stringstream ss;
        ss << "Toggl Desktop - " << value;
        subject_ = ss.str();
    }
    const std::string &Subject() const {
        return subject_;
    }

 private:
    const std::string filename() const;

    std::string subject_;
    std::string details_;
    std::string attachment_path_;
};

}  // namespace toggl

#endif  // SRC_FEEDBACK_H_
