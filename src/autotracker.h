// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_AUTOTRACKER_H_
#define SRC_AUTOTRACKER_H_

#include <string>
#include <sstream>
#include <vector>

#include "Poco/Types.h"

#include "./timeline_event.h"
#include "./base_model.h"

namespace toggl {

class AutotrackerRule : public BaseModel {
 public:
    AutotrackerRule()
        : BaseModel()
    , term_("")
    , pid_(0) {}

    virtual ~AutotrackerRule() {}

    bool Matches(const TimelineEvent event) const;

    const std::string &Term() const;
    void SetTerm(const std::string value);

    const Poco::UInt64 &PID() const;
    void SetPID(const Poco::UInt64 value);

    std::string String() const;
    std::string ModelName() const;
    std::string ModelURL() const;

 private:
    std::string term_;
    Poco::UInt64 pid_;
};

};  // namespace toggl

#endif  // SRC_AUTOTRACKER_H_
