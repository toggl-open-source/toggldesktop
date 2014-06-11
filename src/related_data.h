// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_RELATED_DATA_H_
#define SRC_RELATED_DATA_H_

#include <vector>
#include <set>
#include <string>

#include "./workspace.h"
#include "./client.h"
#include "./project.h"
#include "./task.h"
#include "./tag.h"
#include "./time_entry.h"
#include "./autocomplete_item.h"

namespace kopsik {

class RelatedData {
 public:
    std::vector<Workspace *> Workspaces;
    std::vector<Client *> Clients;
    std::vector<Project *> Projects;
    std::vector<Task *> Tasks;
    std::vector<Tag *> Tags;
    std::vector<TimeEntry *> TimeEntries;

    template<typename T>
    T *modelByID(const Poco::UInt64 id, std::vector<T *> *list) {
        poco_assert(id > 0);
        typedef typename std::vector<T *>::const_iterator iterator;
        for (iterator it = list->begin(); it != list->end(); it++) {
            T *model = *it;
            if (model->ID() == id) {
                return model;
            }
        }
        return 0;
    }

    Task *TaskByID(const Poco::UInt64 id) {
        return modelByID<Task>(id, &Tasks);
    }

    Client *ClientByID(const Poco::UInt64 id) {
        return modelByID(id, &Clients);
    }

    Project *ProjectByID(const Poco::UInt64 id) {
        return modelByID(id, &Projects);
    }

    Tag *TagByID(const Poco::UInt64 id) {
        return modelByID(id, &Tags);
    }

    Workspace *WorkspaceByID(const Poco::UInt64 id) {
        return modelByID(id, &Workspaces);
    }

    TimeEntry *TimeEntryByID(const Poco::UInt64 id) {
        return modelByID(id, &TimeEntries);
    }

    template <typename T>
    T *modelByGUID(const guid GUID, std::vector<T *> *list) {
        if (GUID.empty()) {
            return 0;
        }
        typedef typename std::vector<T *>::const_iterator iterator;
        for (iterator it = list->begin(); it != list->end(); it++) {
            T *model = *it;
            if (model->GUID() == GUID) {
                return model;
            }
        }
        return 0;
    }

    TimeEntry *TimeEntryByGUID(const guid GUID) {
        return modelByGUID(GUID, &TimeEntries);
    }

    Tag *TagByGUID(const guid GUID) {
        return modelByGUID(GUID, &Tags);
    }

    Project *ProjectByGUID(const guid GUID) {
        return modelByGUID(GUID, &Projects);
    }

    Client *ClientByGUID(const guid GUID) {
        return modelByGUID(GUID, &Clients);
    }

    std::vector<AutocompleteItem> AutocompleteItems(
        const bool including_time_entries);

 private:
    void timeEntryAutocompleteItems(
        std::set<std::string> *unique_names,
        std::vector<AutocompleteItem> *list);

    void taskAutocompleteItems(
        std::set<std::string> *unique_names,
        std::vector<AutocompleteItem> *list);

    void projectAutocompleteItems(
        std::set<std::string> *unique_names,
        std::vector<AutocompleteItem> *list);
};

}  // namespace kopsik

#endif  // SRC_RELATED_DATA_H_
