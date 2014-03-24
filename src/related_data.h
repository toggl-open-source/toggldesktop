// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_RELATED_DATA_H_
#define SRC_RELATED_DATA_H_

#include <vector>

#include "./workspace.h"
#include "./client.h"
#include "./project.h"
#include "./task.h"
#include "./tag.h"
#include "./time_entry.h"

namespace kopsik {

class RelatedData {
 public:
    std::vector<Workspace *> Workspaces;
    std::vector<Client *> Clients;
    std::vector<Project *> Projects;
    std::vector<Task *> Tasks;
    std::vector<Tag *> Tags;
    std::vector<TimeEntry *> TimeEntries;
};

}  // namespace kopsik

#endif  // SRC_RELATED_DATA_H_
