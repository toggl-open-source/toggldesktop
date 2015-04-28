
// Copyright 2015 Toggl Desktop developers

#include "../src/autotracker.h"

#include "Poco/UTF8String.h"

namespace toggl {

const std::string &AutotrackerRule::Term() const {
    return term_;
}

void AutotrackerRule::SetTerm(const std::string value) {
    std::string lowercase = Poco::UTF8::toLower(value);
    if (term_ != lowercase) {
        term_ = lowercase;
        SetDirty();
    }
}

const Poco::UInt64 &AutotrackerRule::PID() const {
    return pid_;
}

void AutotrackerRule::SetPID(const Poco::UInt64 value) {
    if (value != pid_) {
        pid_ = value;
        SetDirty();
    }
}

std::string AutotrackerRule::String() const {
    std::stringstream ss;
    ss << " local_id=" << LocalID()
       << " term=" << term_
       << " uid=" << UID()
       << " pid=" << pid_;
    return ss.str();
}

std::string AutotrackerRule::ModelName() const {
    return "autotracker_rule";
}

std::string AutotrackerRule::ModelURL() const {
    return "";
}

Poco::UInt64 Autotracker::FindPID(const TimelineEvent event) const {
    if (rules_.empty()) {
        return 0;
    }
    for (std::vector<AutotrackerRule>::const_iterator it = rules_.begin();
            it != rules_.end(); it++) {
        const AutotrackerRule &rule = *it;
        if (Poco::UTF8::toLower(event.filename).find(rule.Term())
                != std::string::npos) {
            return rule.PID();
        }
        if (Poco::UTF8::toLower(event.title).find(rule.Term())
                != std::string::npos) {
            return rule.PID();
        }
    }
    return 0;
}

}  // namespace toggl
