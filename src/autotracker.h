// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_AUTOTRACKER_H_
#define SRC_AUTOTRACKER_H_

#include <string>
#include <vector>

namespace toggl {

class TimelineEvent;

class AutotrackerRule {
 public:
    AutotrackerRule()
        : local_id_(0)
    , term_("")
    , pid_(0) {}
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
    std::string term_;
    Poco::Int64 local_id_;
    Poco::UInt64 pid_;
};

class Autotracker {
 public:
    Autotracker() {}
    virtual ~Autotracker() {}

    Poco::UInt64 FindPID(TimelineEvent *event);

 private:
    std::vector<AutotrackerRule> rules_;
};

};  // namespace toggl

#endif  // SRC_AUTOTRACKER_H_
