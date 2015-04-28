// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_AUTOTRACKER_H_
#define SRC_AUTOTRACKER_H_

#include <string>
#include <vector>

#include "Poco/Types.h"

#include "./timeline_event.h"

namespace toggl {

class AutotrackerRule {
 public:
    AutotrackerRule(const std::string term, const Poco::UInt64 pid);

    virtual ~AutotrackerRule() {}

    const Poco::Int64 &LocalID() const {
        return local_id_;
    }
    void SetLocalID(const Poco::Int64 value) {
        local_id_ = value;
    }

    const std::string &Term() const {
        return term_;
    }
    void SetTerm(const std::string value) {
        term_ = value;
    }

    const Poco::UInt64 &PID() const {
        return pid_;
    }
    void SetPID(const Poco::UInt64 value) {
        pid_ = value;
    }

 private:
    Poco::Int64 local_id_;
    std::string term_;
    Poco::UInt64 pid_;
};

class Autotracker {
 public:
    Autotracker() {
        // FIXME: add some fake rules
        rules_.push_back(AutotrackerRule("Skype", 4583100));
        rules_.push_back(AutotrackerRule("delfi", 8490176));
    }
    virtual ~Autotracker() {}

    Poco::UInt64 FindPID(const TimelineEvent event) const;

 private:
    std::vector<AutotrackerRule> rules_;
};

};  // namespace toggl

#endif  // SRC_AUTOTRACKER_H_
