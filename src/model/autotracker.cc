
// Copyright 2015 Toggl Desktop developers

#include "model/autotracker.h"

#include <Poco/UTF8String.h>

#include "const.h"

namespace toggl {

bool AutotrackerRule::Matches(const TimelineEvent &event) const {
    if (Poco::UTF8::toLower(event.Filename()).find(Term)
            != std::string::npos) {
        return true;
    }
    if (Poco::UTF8::toLower(event.Title()).find(Term)
            != std::string::npos) {
        return true;
    }
    return false;
}

std::string AutotrackerRule::String() const {
    std::stringstream ss;
    ss << " local_id=" << LocalID()
       << " term=" << Term()
       << " uid=" << UID()
       << " pid=" << PID()
       << " tid=" << TID();
    return ss.str();
}

std::string AutotrackerRule::ModelName() const {
    return kModelAutotrackerRule;
}

std::string AutotrackerRule::ModelURL() const {
    return "";
}

}  // namespace toggl
