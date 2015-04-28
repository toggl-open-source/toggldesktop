
// Copyright 2015 Toggl Desktop developers

#include "../src/autotracker.h"

namespace toggl {

Poco::UInt64 Autotracker::FindPID(TimelineEvent *event) {
    if (!event) {
        return 0;
    }
    if (rules_.empty()) {
        return 0;
    }

    return 0;
}

}  // namespace toggl
