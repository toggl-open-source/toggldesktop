
// Copyright 2015 Toggl Desktop developers

#include "autotracker.h"

#include <Poco/UTF8String.h>

#include "const.h"

namespace toggl {

static const char kTermSeparator = '\t';

bool AutotrackerRule::Matches(const TimelineEvent &event) const {
    const Poco::LocalDateTime localDateTime(Poco::DateTime(event.EndTime()));
    if (!days_of_week_[localDateTime.dayOfWeek()]) {
        return false;
    }

    for (const auto& term : terms_) {
        if (Poco::UTF8::toLower(event.Title()).find(term)
                != std::string::npos) {
            return true;
        }
    }
    return false;
}

const std::vector<std::string>& AutotrackerRule::Terms() const {
    return terms_;
}

void AutotrackerRule::SetTerms(const std::string &value) {
    if (TermsString() != value) {
        terms_.clear();
        if (!value.empty()) {
            std::stringstream ss(value);
            while (ss.good()) {
                std::string tag;
                getline(ss, tag, kTermSeparator);
                terms_.push_back(tag);
            }
        }
        SetDirty();
    }
}

const Poco::UInt64 &AutotrackerRule::PID() const {
    return pid_;
}

void AutotrackerRule::SetPID(const Poco::UInt64 value) {
    if (pid_ != value) {
        pid_ = value;
        SetDirty();
    }
}

const Poco::UInt64 &AutotrackerRule::TID() const {
    return tid_;
}

void AutotrackerRule::SetTID(const Poco::UInt64 value) {
    if (tid_ != value) {
        tid_ = value;
        SetDirty();
    }
}

std::string AutotrackerRule::String() const {
    std::stringstream ss;
    ss << " local_id=" << LocalID()
       << " terms=" << TermsString()
       << " uid=" << UID()
       << " pid=" << pid_
       << " tid=" << tid_;
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
    for (auto it = terms_.begin(); it != terms_.end(); ++it) {
        if (it != terms_.begin()) {
            ss << kTermSeparator;
        }
        ss << *it;
    }
    return ss.str();
}

const std::bitset<7> &AutotrackerRule::DaysOfWeek() const {
    return days_of_week_;
}

void AutotrackerRule::SetDaysOfWeek(const Poco::UInt32 daysOfWeek) {
    days_of_week_ = std::bitset<7>(daysOfWeek);
}

Poco::UInt32 AutotrackerRule::DaysOfWeekUInt32() const {
    return days_of_week_.to_ulong();
}

}  // namespace toggl
