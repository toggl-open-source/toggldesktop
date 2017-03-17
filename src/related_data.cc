// Copyright 2014 Toggl Desktop developers.

#include "../src/related_data.h"

#include <algorithm>
#include <sstream>

#include "Poco/UTF8String.h"

#include "./autotracker.h"
#include "./formatter.h"
#include "./client.h"
#include "./gui.h"
#include "./obm_action.h"
#include "./project.h"
#include "./tag.h"
#include "./task.h"
#include "./time_entry.h"
#include "./workspace.h"

namespace toggl {

template<typename T>
void clearList(std::vector<T *> *list) {
    for (size_t i = 0; i < list->size(); i++) {
        T *value = (*list)[i];
        delete value;
    }
    list->clear();
}

void RelatedData::Clear() {
    clearList(&Workspaces);
    clearList(&Clients);
    clearList(&Projects);
    clearList(&Tasks);
    clearList(&Tags);
    clearList(&TimeEntries);
    clearList(&AutotrackerRules);
    clearList(&TimelineEvents);
    clearList(&ObmActions);
    clearList(&ObmExperiments);
}

error RelatedData::DeleteAutotrackerRule(const Poco::Int64 local_id) {
    if (!local_id) {
        return error("cannot delete rule without an ID");
    }
    for (std::vector<AutotrackerRule *>::iterator it =
        AutotrackerRules.begin();
            it != AutotrackerRules.end(); it++) {
        AutotrackerRule *rule = *it;
        // Autotracker settings are not saved to DB,
        // so the ID will be 0 always. But will have local ID
        if (rule->LocalID() == local_id) {
            rule->MarkAsDeletedOnServer();
            rule->Delete();
            break;
        }
    }
    return noError;
}

AutotrackerRule *RelatedData::FindAutotrackerRule(
    const TimelineEvent event) const {
    for (std::vector<AutotrackerRule *>::const_iterator it =
        AutotrackerRules.begin();
            it != AutotrackerRules.end(); it++) {
        AutotrackerRule *rule = *it;
        if (rule->Matches(event)) {
            return rule;
        }
    }
    return nullptr;
}

bool RelatedData::HasMatchingAutotrackerRule(
    const std::string lowercase_term) const {
    for (std::vector<AutotrackerRule *>::const_iterator it =
        AutotrackerRules.begin();
            it != AutotrackerRules.end(); it++) {
        AutotrackerRule *rule = *it;
        if (rule->Term() == lowercase_term) {
            return true;
        }
    }
    return false;
}

Poco::Int64 RelatedData::NumberOfUnsyncedTimeEntries() const {
    Poco::Int64 count(0);

    for (std::vector<TimeEntry *>::const_iterator it =
        TimeEntries.begin();
            it != TimeEntries.end(); it++) {
        TimeEntry *te = *it;
        if (te->NeedsPush()) {
            count++;
        }
    }

    return count;
}

std::vector<TimelineEvent *> RelatedData::VisibleTimelineEvents() const {
    std::vector<TimelineEvent *> result;
    for (std::vector<TimelineEvent *>::const_iterator i =
        TimelineEvents.begin();
            i != TimelineEvents.end();
            ++i) {
        TimelineEvent *event = *i;
        if (event && event->VisibleToUser()) {
            result.push_back(event);
        }
    }
    return result;
}

std::vector<TimeEntry *> RelatedData::VisibleTimeEntries() const {
    std::vector<TimeEntry *> result;
    for (std::vector<TimeEntry *>::const_iterator it =
        TimeEntries.begin();
            it != TimeEntries.end(); it++) {
        TimeEntry *te = *it;
        if (te->GUID().empty()) {
            continue;
        }
        if (te->DeletedAt() > 0) {
            continue;
        }
        result.push_back(te);
    }
    return result;
}

Poco::Int64 RelatedData::TotalDurationForDate(const TimeEntry *match) const {
    std::string date_header = Formatter::FormatDateHeader(match->Start());
    Poco::Int64 duration(0);
    for (std::vector<TimeEntry *>::const_iterator it =
        TimeEntries.begin();
            it != TimeEntries.end(); it++) {
        TimeEntry *te = *it;
        if (te->GUID().empty()) {
            continue;
        }
        if (te->DeletedAt() > 0) {
            continue;
        }
        if (Formatter::FormatDateHeader(te->Start()) == date_header) {
            duration += Formatter::AbsDuration(te->Duration());
        }
    }
    return duration;
}

TimeEntry *RelatedData::LatestTimeEntry() const {
    TimeEntry *latest = nullptr;
    std::string pomodoro_decription("Pomodoro Break");
    std::string pomodoro_tag("pomodoro-break");

    // Find the time entry that was stopped most recently
    for (std::vector<TimeEntry *>::const_iterator it =
        TimeEntries.begin();
            it != TimeEntries.end(); it++) {
        TimeEntry *te = *it;

        if (te->GUID().empty()) {
            continue;
        }
        if (te->DurationInSeconds() < 0) {
            continue;
        }
        if (te->DeletedAt() > 0) {
            continue;
        }
        if (te->Description().compare(pomodoro_decription) == 0) {
            continue;
        }
        if (te->Tags().find(pomodoro_tag) == 0) {
            continue;
        }

        if (!latest || (te->Stop() > latest->Stop())) {
            latest = te;
        }
    }

    return latest;
}

// Add time entries, in format:
// Description - Task. Project. Client
void RelatedData::timeEntryAutocompleteItems(
    std::set<std::string> *unique_names,
    std::vector<view::Autocomplete> *list) const {

    poco_check_ptr(list);

    for (std::vector<TimeEntry *>::const_iterator it =
        TimeEntries.begin();
            it != TimeEntries.end(); it++) {
        TimeEntry *te = *it;

        if (te->DeletedAt() || te->IsMarkedAsDeletedOnServer()
                || te->Description().empty()) {
            continue;
        }

        Task *t = nullptr;
        if (te->TID()) {
            t = TaskByID(te->TID());
        }

        Project *p = nullptr;
        if (t && t->PID()) {
            p = ProjectByID(t->PID());
        } else if (te->PID()) {
            p = ProjectByID(te->PID());
        }

        if (p && !p->Active()) {
            continue;
        }

        Client *c = clientByProject(p);

        std::string project_task_label =
            Formatter::JoinTaskName(t, p, c);

        std::string description = te->Description();

        std::stringstream search_parts;
        search_parts << te->Description();
        if (!project_task_label.empty()) {
            search_parts << " - " << project_task_label;
        }

        std::string text = search_parts.str();
        if (text.empty()) {
            continue;
        }

        if (unique_names->find(text) != unique_names->end()) {
            continue;
        }
        unique_names->insert(text);

        view::Autocomplete autocomplete_item;
        autocomplete_item.Text = text;
        autocomplete_item.Description = description;
        autocomplete_item.ProjectAndTaskLabel = project_task_label;
        if (p) {
            autocomplete_item.ProjectColor = p->ColorCode();
            autocomplete_item.ProjectID = p->ID();
            autocomplete_item.ProjectLabel = p->Name();
        }
        if (c) {
            autocomplete_item.ClientLabel = c->Name();
            autocomplete_item.ClientID = c->ID();
        }
        if (t) {
            autocomplete_item.TaskID = t->ID();
            autocomplete_item.TaskLabel = t->Name();
        }
        autocomplete_item.WorkspaceID = te->WID();
        autocomplete_item.Tags = te->Tags();
        autocomplete_item.Type = kAutocompleteItemTE;
        autocomplete_item.Billable = te->Billable();
        list->push_back(autocomplete_item);
    }
}

// Add tasks, in format:
// Task. Project. Client
void RelatedData::taskAutocompleteItems(
    std::set<std::string> *unique_names,
    std::map<Poco::UInt64, std::string> *ws_names,
    std::vector<toggl::view::Autocomplete> *list) const {

    poco_check_ptr(list);

    for (std::vector<Task *>::const_iterator it =
        Tasks.begin();
            it != Tasks.end(); it++) {
        Task *t = *it;

        if (t->IsMarkedAsDeletedOnServer()) {
            continue;
        }

        if (!t->Active()) {
            continue;
        }

        Project *p = nullptr;
        if (t->PID()) {
            p = ProjectByID(t->PID());
        }

        if (p && !p->Active()) {
            continue;
        }

        Client *c = clientByProject(p);

        std::string text = Formatter::JoinTaskName(t, p, c);
        if (text.empty()) {
            continue;
        }

        if (unique_names->find(text) != unique_names->end()) {
            continue;
        }
        unique_names->insert(text);

        view::Autocomplete autocomplete_item;
        autocomplete_item.Text = text;
        autocomplete_item.ProjectAndTaskLabel = text;
        autocomplete_item.TaskLabel = t->Name();
        autocomplete_item.TaskID = t->ID();
        if (c) {
            autocomplete_item.ClientLabel = c->Name();
            autocomplete_item.ClientID = c->ID();
        }
        if (p) {
            autocomplete_item.ProjectColor = p->ColorCode();
            autocomplete_item.ProjectID = p->ID();
            autocomplete_item.ProjectLabel = p->Name();
        }
        if (ws_names) {
            autocomplete_item.WorkspaceName = (*ws_names)[t->WID()];
        }
        autocomplete_item.WorkspaceID = t->WID();
        autocomplete_item.Type = kAutocompleteItemTask;
        list->push_back(autocomplete_item);
    }
}

// Add projects, in format:
// Project. Client
void RelatedData::projectAutocompleteItems(
    std::set<std::string> *unique_names,
    std::map<Poco::UInt64, std::string> *ws_names,
    std::vector<view::Autocomplete> *list) const {

    poco_check_ptr(list);

    for (std::vector<Project *>::const_iterator it =
        Projects.begin();
            it != Projects.end(); it++) {
        Project *p = *it;

        if (!p->Active()) {
            continue;
        }

        Client *c = clientByProject(p);

        std::string text = Formatter::JoinTaskName(0, p, c);
        if (text.empty()) {
            continue;
        }

        {
            std::stringstream ss;
            ss << p->WID() << "/" << text;
            std::string uniq_name = ss.str();
            if (unique_names->find(uniq_name) != unique_names->end()) {
                continue;
            }
            unique_names->insert(uniq_name);
        }

        view::Autocomplete autocomplete_item;
        autocomplete_item.Text = text;
        autocomplete_item.ProjectAndTaskLabel = text;
        autocomplete_item.ProjectLabel = p->Name();
        if (c) {
            autocomplete_item.ClientLabel = c->Name();
            autocomplete_item.ClientID = c->ID();
        }
        autocomplete_item.ProjectID = p->ID();
        autocomplete_item.ProjectColor = p->ColorCode();
        if (ws_names) {
            autocomplete_item.WorkspaceName = (*ws_names)[p->WID()];
        }
        autocomplete_item.WorkspaceID = p->WID();
        autocomplete_item.Type = kAutocompleteItemProject;

        list->push_back(autocomplete_item);
    }
}

void RelatedData::TimeEntryAutocompleteItems(
    std::vector<view::Autocomplete> *result) const {
    std::set<std::string> unique_names;

    timeEntryAutocompleteItems(&unique_names, result);
    std::sort(result->begin(), result->end(), CompareAutocompleteItems);
}

void RelatedData::MinitimerAutocompleteItems(
    std::vector<view::Autocomplete> *result) const {
    std::set<std::string> unique_names;

    timeEntryAutocompleteItems(&unique_names, result);
    taskAutocompleteItems(&unique_names, nullptr, result);
    projectAutocompleteItems(&unique_names, nullptr, result);

    std::sort(result->begin(), result->end(), CompareAutocompleteItems);
}

void RelatedData::ProjectAutocompleteItems(
    std::vector<view::Autocomplete> *result) const {
    std::set<std::string> unique_names;

    std::map<Poco::UInt64, std::string> ws_names;
    workspaceAutocompleteItems(&unique_names, &ws_names, result);
    projectAutocompleteItems(&unique_names, &ws_names, result);
    taskAutocompleteItems(&unique_names, &ws_names, result);

    std::sort(result->begin(), result->end(),
              CompareStructuredAutocompleteItems);
}

void RelatedData::workspaceAutocompleteItems(
    std::set<std::string> *unique_names,
    std::map<Poco::UInt64, std::string> *ws_names,
    std::vector<view::Autocomplete> *list) const {

    // remember workspaces that have projects
    std::set<Poco::UInt64> ws_ids_with_projects;
    for (std::vector<Project *>::const_iterator it =
        Projects.begin();
            it != Projects.end(); it++) {
        Project *p = *it;

        if (p->Active()) {
            ws_ids_with_projects.insert(p->WID());
        }
    }

    for (std::vector<Workspace *>::const_iterator it =
        Workspaces.begin();
            it != Workspaces.end(); it++) {
        Workspace *ws = *it;

        if (ws_ids_with_projects.find(ws->ID()) == ws_ids_with_projects.end()) {
            continue;
        }

        std::string ws_name = Poco::UTF8::toUpper(ws->Name());
        (*ws_names)[ws->ID()] = ws_name;

        view::Autocomplete autocomplete_item;
        autocomplete_item.Text = ws_name;
        autocomplete_item.WorkspaceName = ws_name;
        autocomplete_item.WorkspaceID = ws->ID();
        autocomplete_item.Type = kAutocompleteItemWorkspace;
        list->push_back(autocomplete_item);
    }
}

void RelatedData::TagList(
    std::vector<std::string> *tags,
    const Poco::UInt64 wid) const {
    std::set<std::string> unique_names;

    for (std::vector<Tag *>::const_iterator it =
        Tags.begin();
            it != Tags.end();
            it++) {
        Tag *tag = *it;
        if (wid && tag->WID() != wid) {
            continue;
        }
        if (unique_names.find(tag->Name()) != unique_names.end()) {
            continue;
        }
        unique_names.insert(tag->Name());
        tags->push_back(tag->Name());
    }

    std::sort(tags->rbegin(), tags->rend());
}

void RelatedData::WorkspaceList(std::vector<Workspace *> *result) const {
    for (std::vector<Workspace *>::const_iterator it =
        Workspaces.begin();
            it != Workspaces.end();
            it++) {
        Workspace *ws = *it;
        if (!ws->Admin() && ws->OnlyAdminsMayCreateProjects()) {
            continue;
        }
        result->push_back(ws);
    }
    std::sort(result->rbegin(), result->rend(), CompareWorkspaceByName);
}

void RelatedData::ClientList(std::vector<Client *> *result) const {
    *result = Clients;

    std::sort(result->rbegin(), result->rend(), CompareClientByName);
}

void RelatedData::ProjectLabelAndColorCode(
    TimeEntry * const te,
    view::TimeEntry *view) const {

    poco_check_ptr(te);
    poco_check_ptr(view);

    Workspace *ws = nullptr;
    if (te->WID()) {
        ws = WorkspaceByID(te->WID());
    }
    if (ws) {
        view->WorkspaceName = ws->Name();
    }

    Task *t = nullptr;
    if (te->TID()) {
        t = TaskByID(te->TID());
    }
    if (t) {
        view->TaskLabel = t->Name();
    }

    Project *p = nullptr;
    if (t && t->PID()) {
        p = ProjectByID(t->PID());
    }
    if (!p && te->PID()) {
        p = ProjectByID(te->PID());
    }
    if (!p && !te->ProjectGUID().empty()) {
        p = ProjectByGUID(te->ProjectGUID());
    }

    Client *c = clientByProject(p);

    view->ProjectAndTaskLabel = Formatter::JoinTaskName(t, p, c);

    if (p) {
        view->Color = p->ColorCode();
        view->ProjectLabel = p->Name();
    }

    if (c) {
        view->ClientLabel = c->Name();
    }
}

Client *RelatedData::clientByProject(Project *p) const {
    Client *c = nullptr;
    if (p && p->CID()) {
        c = ClientByID(p->CID());
    }
    if (!c && p && !p->ClientGUID().empty()) {
        c = ClientByGUID(p->ClientGUID());
    }
    return c;
}

Task *RelatedData::TaskByID(const Poco::UInt64 id) const {
    return modelByID<Task>(id, &Tasks);
}

Client *RelatedData::ClientByID(const Poco::UInt64 id) const {
    return modelByID(id, &Clients);
}

Project *RelatedData::ProjectByID(const Poco::UInt64 id) const {
    return modelByID(id, &Projects);
}

Tag *RelatedData::TagByID(const Poco::UInt64 id) const {
    return modelByID(id, &Tags);
}

Workspace *RelatedData::WorkspaceByID(const Poco::UInt64 id) const {
    return modelByID(id, &Workspaces);
}

TimeEntry *RelatedData::TimeEntryByID(const Poco::UInt64 id) const {
    return modelByID(id, &TimeEntries);
}

TimeEntry *RelatedData::TimeEntryByGUID(const guid GUID) const {
    return modelByGUID(GUID, &TimeEntries);
}

TimelineEvent *RelatedData::TimelineEventByGUID(const guid GUID) const {
    return modelByGUID(GUID, &TimelineEvents);
}

Tag *RelatedData::TagByGUID(const guid GUID) const {
    return modelByGUID(GUID, &Tags);
}

Project *RelatedData::ProjectByGUID(const guid GUID) const {
    return modelByGUID(GUID, &Projects);
}

Client *RelatedData::ClientByGUID(const guid GUID) const {
    return modelByGUID(GUID, &Clients);
}

template <typename T>
T *modelByGUID(const guid GUID, std::vector<T *> const *list) {
    if (GUID.empty()) {
        return nullptr;
    }
    typedef typename std::vector<T *>::const_iterator iterator;
    for (iterator it = list->begin(); it != list->end(); it++) {
        T *model = *it;
        if (model->GUID() == GUID) {
            return model;
        }
    }
    return nullptr;
}

template<typename T>
T *modelByID(const Poco::UInt64 id, std::vector<T *> const *list) {
    if (!id) {
        return nullptr;
    }
    typedef typename std::vector<T *>::const_iterator iterator;
    for (iterator it = list->begin(); it != list->end(); it++) {
        T *model = *it;
        if (model->ID() == id) {
            return model;
        }
    }
    return nullptr;
}

}   // namespace toggl
