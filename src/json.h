
// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_JSON_H_
#define SRC_JSON_H_

#include <string>
#include <set>
#include <vector>
#include <map>

#include <json/json.h>  // NOLINT

#include "./workspace.h"
#include "./client.h"
#include "./project.h"
#include "./task.h"
#include "./time_entry.h"
#include "./tag.h"
#include "./batch_update_result.h"
#include "./timeline_event.h"

namespace toggl {

namespace json {

std::string UpdateJSON(
    std::vector<Project *> * const,
    std::vector<TimeEntry *> * const);

std::string ConvertTimelineToJSON(
    const std::vector<TimelineEvent> &timeline_events,
    const std::string &desktop_id);

}  // namespace json

}  // namespace toggl

#endif  // SRC_JSON_H_
