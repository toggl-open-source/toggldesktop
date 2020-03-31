
// Copyright 2015 Toggl Desktop developers

#include "model/autotracker.h"

#include <Poco/UTF8String.h>

#include "const.h"

namespace toggl {

static const char kTermSeparator = '\t';

bool AutotrackerRule::Matches(const TimelineEvent &event) const {
    for (const auto& term : Terms()) {
        if (Poco::UTF8::toLower(event.Title()).find(term)
                != std::string::npos) {
            return true;
        }
    }
    return false;
}

void AutotrackerRule::SetTerms(const std::string &value) {
    if (TermsString() != value) {
        Terms().clear();
        if (!value.empty()) {
            std::stringstream ss(value);
            while (ss.good()) {
                std::string tag;
                getline(ss, tag, kTermSeparator);
                Terms().push_back(tag);
            }
        }
        SetDirty();
}

void AutotrackerRule::SetPID(Poco::UInt64 value) {
    if (PID.Set(value))
        SetDirty();
}

void AutotrackerRule::SetTID(Poco::UInt64 value) {
    if (TID.Set(value))
        SetDirty();
}

std::string AutotrackerRule::String() const {
    std::stringstream ss;
    ss << " local_id=" << LocalID()
       << " terms=" << TermsString()
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

const std::string AutotrackerRule::TermsString() const {
    std::stringstream ss;
    for (auto it = Terms().begin(); it != Terms().end(); ++it) {
        if (it != Terms().begin()) {
            ss << kTermSeparator;
        }
        ss << *it;
    }
    return ss.str();
}

}  // namespace toggl
