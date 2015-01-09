// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_RELATED_DATA_H_
#define SRC_RELATED_DATA_H_

#include <vector>
#include <set>
#include <string>
#include <map>

#include "./autocomplete_item.h"
#include "./types.h"

namespace toggl {

class Client;
class Project;
class Tag;
class Task;
class TimeEntry;
class Workspace;

template<typename T>
T *modelByID(const Poco::UInt64 id, std::vector<T *> const *list);

template <typename T>
T *modelByGUID(const guid GUID, std::vector<T *> const *list);

class RelatedData {
 public:
    std::vector<Workspace *> Workspaces;
    std::vector<Client *> Clients;
    std::vector<Project *> Projects;
    std::vector<Task *> Tasks;
    std::vector<Tag *> Tags;
    std::vector<TimeEntry *> TimeEntries;

    Task *TaskByID(const Poco::UInt64 id) const;
    Client *ClientByID(const Poco::UInt64 id) const;
    Project *ProjectByID(const Poco::UInt64 id) const;
    Tag *TagByID(const Poco::UInt64 id) const;
    Workspace *WorkspaceByID(const Poco::UInt64 id) const;
    TimeEntry *TimeEntryByID(const Poco::UInt64 id) const;

    std::vector<std::string> TagList() const;
    std::vector<Workspace *> WorkspaceList() const;
    std::vector<Client *> ClientList() const;

    TimeEntry *TimeEntryByGUID(const guid GUID) const;
    Tag *TagByGUID(const guid GUID) const;
    Project *ProjectByGUID(const guid GUID) const;
    Client *ClientByGUID(const guid GUID) const;

    std::vector<AutocompleteItem> TimeEntryAutocompleteItems();

    std::vector<AutocompleteItem> MinitimerAutocompleteItems();

    std::vector<AutocompleteItem> ProjectAutocompleteItems();

    std::vector<AutocompleteItem> StructuredAutocompleteItems();

    void ProjectLabelAndColorCode(
        TimeEntry *te,
        std::string *workspace_name,
        std::string *project_and_task_label,
        std::string *task_label,
        std::string *project_label,
        std::string *client_label,
        std::string *color_code) const;

 private:
    void timeEntryAutocompleteItems(
        std::set<std::string> *unique_names,
        std::vector<AutocompleteItem> *list);

    void taskAutocompleteItems(
        std::set<std::string> *unique_names,
        std::map<Poco::UInt64, std::string> *ws_names,
        std::vector<AutocompleteItem> *list);

    void projectAutocompleteItems(
        std::set<std::string> *unique_names,
        std::map<Poco::UInt64, std::string> *ws_names,
        std::vector<AutocompleteItem> *list);

    void workspaceAutocompleteItems(
        std::set<std::string> *unique_names,
        std::map<Poco::UInt64, std::string> *ws_names,
        std::vector<AutocompleteItem> *list);
};

}  // namespace toggl

#endif  // SRC_RELATED_DATA_H_
