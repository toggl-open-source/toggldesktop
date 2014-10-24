// Copyright 2014 Toggl Desktop developers.

#include "./json.h"

#include <sstream>
#include <cstring>

#include "./formatter.h"

namespace toggl {

namespace json {

std::string UpdateJSON(
    std::vector<Project *> * const projects,
    std::vector<TimeEntry *> * const time_entries) {

    poco_check_ptr(projects);
    poco_check_ptr(time_entries);

    Json::Value c;

    // First, projects, because time entries depend on projects
    for (std::vector<Project *>::const_iterator it =
        projects->begin();
            it != projects->end(); it++) {
        c.append((*it)->BatchUpdateJSON());
    }

    // Time entries go last
    for (std::vector<TimeEntry *>::const_iterator it =
        time_entries->begin();
            it != time_entries->end(); it++) {
        c.append((*it)->BatchUpdateJSON());
    }

    Json::StyledWriter writer;
    return writer.write(c);
}

std::string ConvertTimelineToJSON(
    const std::vector<TimelineEvent> &timeline_events,
    const std::string &desktop_id) {

    Json::Value root;

    for (std::vector<TimelineEvent>::const_iterator i = timeline_events.begin();
            i != timeline_events.end();
            ++i) {
        const TimelineEvent &event = *i;
        // initialize new event node
        Json::Value n;
        // add fields to event node
        if (event.idle) {
            n["idle"] = true;
        } else {
            n["filename"] = event.filename;
            n["title"] = event.title;
        }
        n["start_time"] = Json::Int64(event.start_time);
        n["end_time"] = Json::Int64(event.end_time);
        n["desktop_id"] = desktop_id;
        n["created_with"] = "timeline";

        // Push event node to array
        root.append(n);
    }

    Json::StyledWriter writer;
    return writer.write(root);
}

}   // namespace json

}   // namespace toggl
