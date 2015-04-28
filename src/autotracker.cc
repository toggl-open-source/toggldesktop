
// Copyright 2015 Toggl Desktop developers

#include "../src/autotracker.h"

#include "Poco/UTF8String.h"

namespace toggl {

AutotrackerRule::AutotrackerRule(const std::string term, const Poco::UInt64 pid)
    : local_id_(0)
, term_(Poco::UTF8::toLower(term))
, pid_(pid) {
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
