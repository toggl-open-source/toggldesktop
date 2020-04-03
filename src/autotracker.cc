
// Copyright 2015 Toggl Desktop developers

#include "autotracker.h"

#include <Poco/UTF8String.h>

#include "const.h"

namespace toggl {

static const char kTermSeparator = '\t';

bool AutotrackerRule::Matches(const TimelineEvent &event) const {
    const Poco::LocalDateTime event_time(Poco::DateTime(event.EndTime()));
    if (!days_of_week_.none() && !days_of_week_[event_time.dayOfWeek()]) {
        logger().debug("Autotracker rule is not enabled on this weekday");
        return false;
    }
    if (!start_time_.empty()) {
        int h(0), m(0);
        if (toggl::Formatter::ParseTimeInput(start_time_, &h, &m)) {
            Poco::LocalDateTime start(
                event_time.year(), event_time.month(), event_time.day(), h, m, event_time.second());
            if (event_time < start) {
                logger().debug("It's too early for this autotracker rule", " [", event_time.hour(), ":", event_time.minute(), "]", " (allowed from ", h, ":", m, ")");
                return false;
            }
        }
    }

    if (!end_time_.empty()) {
        int h(0), m(0);
        if (toggl::Formatter::ParseTimeInput(end_time_, &h, &m)) {
            Poco::LocalDateTime end(
                event_time.year(), event_time.month(), event_time.day(), h, m, event_time.second());
            if (event_time > end) {
                logger().debug("It's too late for this autotracker rule", " [", event_time.hour(), ":", event_time.minute(), "]", " (allowed until ", h, ":", m, ")");
                return false;
            }
        }
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

const std::string &AutotrackerRule::StartTime() const {
    return start_time_;
}

void AutotrackerRule::SetStartTime(const std::string &value) {
    if (start_time_ != value) {
        start_time_ = value;
        SetDirty();
    }
}

const std::string &AutotrackerRule::EndTime() const {
    return end_time_;
}

void AutotrackerRule::SetEndTime(const std::string &value) {
    if (end_time_ != value) {
        end_time_ = value;
        SetDirty();
    }
}

}  // namespace toggl
