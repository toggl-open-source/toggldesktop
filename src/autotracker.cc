
// Copyright 2015 Toggl Desktop developers

#include "../src/autotracker.h"

#include "Poco/UTF8String.h"

#include "./const.h"

namespace toggl {

bool AutotrackerRule::Matches(const TimelineEvent event) const {
    if (Poco::UTF8::toLower(event.filename).find(term_)
            != std::string::npos) {
        return true;
    }
    if (Poco::UTF8::toLower(event.title).find(term_)
            != std::string::npos) {
        return true;
    }
    return false;
}

const std::string &AutotrackerRule::Term() const {
    return term_;
}

void AutotrackerRule::SetTerm(const std::string value) {
    if (term_ != value) {
        term_ = value;
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
    return kModelAutotrackerRule;
}

std::string AutotrackerRule::ModelURL() const {
    return "";
}

}  // namespace toggl
