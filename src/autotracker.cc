
// Copyright 2015 Toggl Desktop developers

#include "../src/autotracker.h"

namespace toggl {

const Poco::UInt64 Autotracker::FindPID(TimelineEvent *event) const {
    if (!event) {
        return 0;
    }
    if (rules_.empty()) {
        return 0;
    }
    for (std::vector<AutotrackerRule>::const_iterator it = rules_.begin();
            it != rules_.end(); it++) {
        const AutotrackerRule &rule = *it;
        if (Poco::UTF8::toLower(event->filename).find(rule.Term())
                != std::string::npos) {
            return rule.PID();
        }
        if (Poco::UTF8::toLower(event->title).find(rule.Term())
                != std::string::npos) {
            return rule.PID();
        }
    }
    return 0;
}

}  // namespace toggl
