// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_AUTOTRACKER_H_
#define SRC_AUTOTRACKER_H_

#include <string>
#include <sstream>
#include <vector>

#include <Poco/Types.h>

#include "base_model.h"
#include "timeline_event.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT AutotrackerRule : public BaseModel {
 public:
    AutotrackerRule()
        : BaseModel()
    , pid_(0)
    , tid_(0) {}

    virtual ~AutotrackerRule() {}

    bool Matches(const TimelineEvent &event) const;

    const std::vector<std::string> &Terms() const;
    void SetTerms(const std::string &value);
    const std::string TermsString() const;

    const Poco::UInt64 &PID() const;
    void SetPID(const Poco::UInt64 value);

    const Poco::UInt64 &TID() const;
    void SetTID(const Poco::UInt64 value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;

 private:
    std::vector<std::string> terms_;
    Poco::UInt64 pid_;
    Poco::UInt64 tid_;
};

};  // namespace toggl

#endif  // SRC_AUTOTRACKER_H_
