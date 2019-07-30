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
void clearList(std::set<T *> &list) {
    for (auto it : list) {
        delete it;
    }
    list.clear();
}

void RelatedData::forEachTimeEntries(std::function<void(TimeEntry *)> f) {
    auto timeEntries = TimeEntries();
    std::for_each(timeEntries->begin(), timeEntries->end(), f);
}

void RelatedData::pushBackTimeEntry(TimeEntry *timeEntry) {
    auto timeEntries = TimeEntries();
    timeEntries->insert(timeEntry);
}

void RelatedData::Clear() {
    clearList(*Workspaces());
    clearList(*Clients());
    clearList(*Projects());
    clearList(*Tasks());
    clearList(*Tags());
    clearList(*TimeEntries());
    clearList(*AutotrackerRules());
    clearList(*TimelineEvents());
    clearList(*ObmActions());
    clearList(*ObmExperiments());
}

error RelatedData::DeleteAutotrackerRule(const Poco::Int64 local_id) {
    if (!local_id) {
        return error("cannot delete rule without an ID");
    }
    auto autotrackerRules = AutotrackerRules();
    for (auto it = autotrackerRules->begin(); it != autotrackerRules->end(); it++) {
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
    auto autotrackerRules = AutotrackerRules();
    for (auto it = autotrackerRules->begin(); it != autotrackerRules->end(); it++) {
        AutotrackerRule *rule = *it;
        if (rule->Matches(event)) {
            return rule;
        }
    }
    return nullptr;
}

bool RelatedData::HasMatchingAutotrackerRule(
    const std::string lowercase_term) const {
    auto autotrackerRules = AutotrackerRules();
    for (auto it = autotrackerRules->begin(); it != autotrackerRules->end(); it++) {
        AutotrackerRule *rule = *it;
        if (rule->Term() == lowercase_term) {
            return true;
        }
    }
    return false;
}

Poco::Int64 RelatedData::NumberOfUnsyncedTimeEntries() const {
    Poco::Int64 count(0);
    auto timeEntries = TimeEntries();
    for (auto it = timeEntries->begin(); it != timeEntries->end(); it++) {
        TimeEntry *te = *it;
        if (te->NeedsPush()) {
            count++;
        }
    }

    return count;
}

std::vector<TimelineEvent *> RelatedData::VisibleTimelineEvents() const {
    std::vector<TimelineEvent *> result;
    auto timelineEvents = TimelineEvents();
    for (auto it = timelineEvents->begin(); it != timelineEvents->end(); it++) {
        TimelineEvent *event = *it;
        if (event && event->VisibleToUser()) {
            result.push_back(event);
        }
    }
    return result;
}

std::vector<TimeEntry *> RelatedData::VisibleTimeEntries() const {
    std::vector<TimeEntry *> result;
    auto timeEntries = TimeEntries();
    for (auto it = timeEntries->begin(); it != timeEntries->end(); it++) {
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
    auto timeEntries = TimeEntries();
    for (auto it = timeEntries->begin(); it != timeEntries->end(); it++) {
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
    auto timeEntries = TimeEntries();
    for (auto it = timeEntries->begin(); it != timeEntries->end(); it++) {
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
    std::map<Poco::UInt64, std::string> *ws_names,
    std::vector<view::Autocomplete> *list,
    std::map<std::string, std::vector<view::Autocomplete> > *items) const {

    poco_check_ptr(list);

    auto timeEntries = TimeEntries();
    for (auto it = timeEntries->begin(); it != timeEntries->end(); it++) {
        TimeEntry *te = *it;

        if (te->DeletedAt() || te->IsMarkedAsDeletedOnServer()
                || te->Description().empty()) {
            continue;
        }

        const_protected_variable<Task> t;
        if (te->TID()) {
            t = TaskByID(te->TID());
        }

        const_protected_variable<Project> p;
        if (t && t->PID()) {
            p = ProjectByID(t->PID());
        } else if (te->PID()) {
            p = ProjectByID(te->PID());
        }

        if (p && !p->Active()) {
            continue;
        }

        std::string project_task_label =
            Formatter::JoinTaskName(t.data(), p.data());

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
            autocomplete_item.ProjectGUID = p->GUID();
            autocomplete_item.ProjectLabel = p->Name();
            if (p->CID()) {
                autocomplete_item.ClientLabel = p->ClientName();
                autocomplete_item.ClientID = p->CID();
            }
        }

        if (t) {
            autocomplete_item.TaskID = t->ID();
            autocomplete_item.TaskLabel = t->Name();
        }
        autocomplete_item.WorkspaceID = te->WID();
        if (ws_names) {
            autocomplete_item.WorkspaceName = (*ws_names)[te->WID()];
        }
        autocomplete_item.Tags = te->Tags();
        autocomplete_item.Type = kAutocompleteItemTE;
        autocomplete_item.Billable = te->Billable();

        if (items && !autocomplete_item.WorkspaceName.empty()) {
            (*items)[autocomplete_item.WorkspaceName].push_back(autocomplete_item);
        } else {
            list->push_back(autocomplete_item);
        }
    }
}

// Add tasks, in format:
// Task. Project. Client
void RelatedData::taskAutocompleteItems(
    std::set<std::string> *unique_names,
    std::map<Poco::UInt64, std::string> *ws_names,
    std::vector<toggl::view::Autocomplete> *list,
    std::map<Poco::Int64, std::vector<view::Autocomplete> > *items) const {

    poco_check_ptr(list);

    auto tasks = Tasks();
    for (auto it = tasks->begin(); it != tasks->end(); it++) {
        Task *t = *it;

        if (t == NULL) {
            continue;
        }

        if (t->IsMarkedAsDeletedOnServer()) {
            continue;
        }

        if (!t->Active()) {
            continue;
        }

        const_protected_variable<Project> p;
        if (t->PID()) {
            p = ProjectByID(t->PID());
        }

        if (p && !p->Active()) {
            continue;
        }

        std::string text = Formatter::JoinTaskName(t, p.data());
        if (text.empty()) {
            continue;
        }

        if (unique_names->find(text) != unique_names->end()) {
            continue;
        }
        unique_names->insert(text);

        view::Autocomplete autocomplete_item;
        autocomplete_item.Text = t->Name();
        autocomplete_item.ProjectAndTaskLabel = text;
        autocomplete_item.TaskLabel = t->Name();
        autocomplete_item.TaskID = t->ID();

        if (p) {
            autocomplete_item.ProjectColor = p->ColorCode();
            autocomplete_item.ProjectID = p->ID();
            autocomplete_item.ProjectGUID = p->GUID();
            autocomplete_item.ProjectLabel = p->Name();
            autocomplete_item.Billable = p->Billable();
            if (p->CID()) {
                autocomplete_item.ClientLabel = p->ClientName();
                autocomplete_item.ClientID = p->CID();
            }
        }
        if (ws_names) {
            autocomplete_item.WorkspaceName = (*ws_names)[t->WID()];
        }
        autocomplete_item.WorkspaceID = t->WID();
        autocomplete_item.Type = kAutocompleteItemTask;

        if (items) {
            (*items)[autocomplete_item.ProjectID].push_back(autocomplete_item);
        } else {
            list->push_back(autocomplete_item);
        }
    }
}

// Add projects, in format:
// Project. Client
void RelatedData::projectAutocompleteItems(
    std::set<std::string> *unique_names,
    std::map<Poco::UInt64, std::string> *ws_names,
    std::vector<view::Autocomplete> *list,
    std::map<std::string, std::vector<view::Autocomplete> > *items,
    std::map<Poco::Int64, std::vector<view::Autocomplete> > *task_items) const {

    poco_check_ptr(list);

    auto projects = Projects();
    for (auto it = projects->begin(); it != projects->end(); it++) {
        Project *p = *it;

        if (!p->Active()) {
            continue;
        }

        const_protected_variable<Client> c = clientByProject(p);

        std::string text = Formatter::JoinTaskName(0, p);
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
        autocomplete_item.ProjectGUID = p->GUID();
        autocomplete_item.ProjectColor = p->ColorCode();
        if (ws_names) {
            autocomplete_item.WorkspaceName = (*ws_names)[p->WID()];
        }
        autocomplete_item.WorkspaceID = p->WID();
        autocomplete_item.Type = kAutocompleteItemProject;

        if (items && !autocomplete_item.WorkspaceName.empty()) {
            (*items)[autocomplete_item.WorkspaceName].push_back(autocomplete_item);
            if (task_items) {
                for (std::vector<view::Autocomplete>::const_iterator it =
                    (*task_items)[autocomplete_item.ProjectID].begin();
                        it != (*task_items)[autocomplete_item.ProjectID].end(); it++) {
                    view::Autocomplete ac = *it;
                    (*items)[autocomplete_item.WorkspaceName].push_back(ac);
                }
            }
        } else {
            list->push_back(autocomplete_item);
            if (task_items) {
                for (std::vector<view::Autocomplete>::const_iterator it =
                    (*task_items)[autocomplete_item.ProjectID].begin();
                        it != (*task_items)[autocomplete_item.ProjectID].end(); it++) {
                    view::Autocomplete ac = *it;
                    list->push_back(ac);
                }
            }
        }
    }
}

void RelatedData::TimeEntryAutocompleteItems(
    std::vector<view::Autocomplete> *result) const {
    std::set<std::string> unique_names;
    std::map<Poco::UInt64, std::string> ws_names;
    std::map<std::string, std::vector<view::Autocomplete> > items;
    workspaceAutocompleteItems(&unique_names, &ws_names, result);
    timeEntryAutocompleteItems(&unique_names, &ws_names, result, &items);
    mergeGroupedAutocompleteItems(result, &items);
}

void RelatedData::MinitimerAutocompleteItems(
    std::vector<view::Autocomplete> *result) const {
    std::set<std::string> unique_names;
    std::map<Poco::UInt64, std::string> ws_names;
    std::map<std::string, std::vector<view::Autocomplete> > items;
    std::map<Poco::Int64, std::vector<view::Autocomplete> > task_items;

    workspaceAutocompleteItems(&unique_names, &ws_names, result);
    timeEntryAutocompleteItems(&unique_names, &ws_names, result, &items);
    taskAutocompleteItems(&unique_names, &ws_names, result, &task_items);
    projectAutocompleteItems(&unique_names, &ws_names, result, &items, &task_items);

    mergeGroupedAutocompleteItems(result, &items);
}

void RelatedData::mergeGroupedAutocompleteItems(
    std::vector<view::Autocomplete> *result,
    std::map<std::string, std::vector<view::Autocomplete> > *items) const {
    // Join created workspace maps to a single vector
    Poco::UInt64 total_size = 0;
    for(std::map<std::string, std::vector<view::Autocomplete> >::iterator iter =
        items->begin(); iter != items->end(); ++iter)
    {
        total_size += iter->second.size();
    }

    result->reserve(total_size);

    for(std::map<std::string, std::vector<view::Autocomplete> >::iterator iter =
        items->begin(); iter != items->end(); ++iter)
    {
        result->insert(result->end(), iter->second.begin(), iter->second.end());
    }
}


void RelatedData::ProjectAutocompleteItems(
    std::vector<view::Autocomplete> *result) const {
    std::set<std::string> unique_names;
    std::map<Poco::UInt64, std::string> ws_names;
    std::map<Poco::Int64, std::vector<view::Autocomplete> > task_items;
    workspaceAutocompleteItems(&unique_names, &ws_names, result);
    taskAutocompleteItems(&unique_names, &ws_names, result, &task_items);
    projectAutocompleteItems(&unique_names, &ws_names, result, nullptr, &task_items);
}

void RelatedData::workspaceAutocompleteItems(
    std::set<std::string> *unique_names,
    std::map<Poco::UInt64, std::string> *ws_names,
    std::vector<view::Autocomplete> *list) const {

    // remember workspaces that have projects
    std::set<Poco::UInt64> ws_ids_with_projects;
    auto projects = Projects();
    for (auto it = projects->begin(); it != projects->end(); it++) {
        Project *p = *it;

        if (p->Active()) {
            ws_ids_with_projects.insert(p->WID());
        }
    }

    auto workspaces = Workspaces();
    for (auto it = workspaces->begin(); it != workspaces->end(); it++) {
        Workspace *ws = *it;

        if (ws_ids_with_projects.find(ws->ID()) == ws_ids_with_projects.end()) {
            continue;
        }

        std::string ws_name = Poco::UTF8::toUpper(ws->Name());
        (*ws_names)[ws->ID()] = ws_name;
    }
}

void RelatedData::TagList(
    std::vector<std::string> *tag_names,
    const Poco::UInt64 wid) const {

    poco_check_ptr(tag_names);

    std::set<std::string> unique_names;

    auto tags = Tags();
    for (auto it = tags->begin(); it != tags->end(); it++) {
        Tag *tag = *it;
        if (wid && tag->WID() != wid) {
            continue;
        }
        if (unique_names.find(tag->Name()) != unique_names.end()) {
            continue;
        }
        unique_names.insert(tag->Name());
        tag_names->push_back(tag->Name());
    }

    // OVERHAUL_TODO
    //std::sort(tags->rbegin(), tags->rend());
}

void RelatedData::WorkspaceList(std::vector<Workspace *> *result) const {

    poco_check_ptr(result);

    auto workspaces = Workspaces();
    for (auto it = workspaces->begin(); it != workspaces->end(); it++) {
        Workspace *ws = *it;
        if (!ws->Admin() && ws->OnlyAdminsMayCreateProjects()) {
            continue;
        }
        result->push_back(ws);
    }

    // OVERHAUL_TODO
    //std::sort(result->rbegin(), result->rend(), CompareWorkspaceByName);
}

void RelatedData::ClientList(std::vector<Client *> *result) const {

    poco_check_ptr(result);
    auto clients = Clients();
    result->clear();
    for (auto i : *clients) {
        result->push_back(i);
    }

    // OVERHAUL_TODO
    //std::sort(result->rbegin(), result->rend(), CompareClientByName);
}

void RelatedData::ProjectLabelAndColorCode(
    TimeEntry * const te,
    view::TimeEntry *view) const {

    poco_check_ptr(te);
    poco_check_ptr(view);

    const_protected_variable<Workspace> ws;
    if (te->WID()) {
        ws = WorkspaceByID(te->WID());
    }
    if (ws) {
        view->WorkspaceName = ws->Name();
    }

    const_protected_variable<Task> t;
    if (te->TID()) {
        t = TaskByID(te->TID());
    }
    if (t) {
        view->TaskLabel = t->Name();
    }

    const_protected_variable<Project> p;
    if (t && t->PID()) {
        p = ProjectByID(t->PID());
    }
    if (!p && te->PID()) {
        p = ProjectByID(te->PID());
    }
    if (!p && !te->ProjectGUID().empty()) {
        p = ProjectByGUID(te->ProjectGUID());
    }

    const_protected_variable<Client> c = clientByProject(p.data());

    view->ProjectAndTaskLabel = Formatter::JoinTaskName(t.data(), p.data());

    if (p) {
        view->Color = p->ColorCode();
        view->ProjectLabel = p->Name();
    }

    if (c) {
        view->ClientLabel = c->Name();
    }
}

protected_variable<Client> RelatedData::clientByProject(Project *p) {
    protected_variable<Client> c;
    if (p && p->CID()) {
        c = ClientByID(p->CID());
    }
    if (!c && p && !p->ClientGUID().empty()) {
        c = ClientByGUID(p->ClientGUID());
    }
    return c;
}

const_protected_variable<Client> RelatedData::clientByProject(const Project *p) const {
    const_protected_variable<Client> c;
    if (p && p->CID()) {
        c = ClientByID(p->CID());
    }
    if (!c && p && !p->ClientGUID().empty()) {
        c = ClientByGUID(p->ClientGUID());
    }
    return c;
}

protected_variable<Task> RelatedData::TaskByID(const Poco::UInt64 id) {
    auto tasks = Tasks();
    return { tasks_m_, modelByID<Task>(id, *tasks) };
}

protected_variable<Client> RelatedData::ClientByID(const Poco::UInt64 id) {
    auto clients = Clients();
    return { clients_m_, modelByID(id, *clients) };
}

protected_variable<Project> RelatedData::ProjectByID(const Poco::UInt64 id) {
    auto projects = Projects();
    return { projects_m_, modelByID(id, *projects) };
}

protected_variable<Tag> RelatedData::TagByID(const Poco::UInt64 id) {
    auto tags = Tags();
    return { tags_m_, modelByID(id, *tags) };
}

protected_variable<Workspace> RelatedData::WorkspaceByID(const Poco::UInt64 id) {
    auto workspaces = Workspaces();
    return { workspaces_m_, modelByID(id, *workspaces) };
}

protected_variable<TimeEntry> RelatedData::TimeEntryByID(const Poco::UInt64 id) {
    auto timeEntries = TimeEntries();
    return { timeEntries_m_, modelByID(id, *timeEntries) };
}

protected_variable<TimeEntry> RelatedData::TimeEntryByGUID(const guid GUID) {
    auto timeEntries = TimeEntries();
    return { timeEntries_m_, modelByGUID(GUID, *timeEntries) };
}

protected_variable<TimelineEvent> RelatedData::TimelineEventByGUID(const guid GUID) {
    auto timelineEvents = TimelineEvents();
    return { timelineEvents_m_, modelByGUID(GUID, *timelineEvents) };
}

protected_variable<Tag> RelatedData::TagByGUID(const guid GUID) {
    auto tags = Tags();
    return { tags_m_, modelByGUID(GUID, *tags) };
}

protected_variable<Project> RelatedData::ProjectByGUID(const guid GUID) {
    auto projects = Projects();
    return { projects_m_, modelByGUID(GUID, *projects) };
}

protected_variable<Client> RelatedData::ClientByGUID(const guid GUID) {
    auto clients = Clients();
    return { clients_m_, modelByGUID(GUID, *clients) };
}

const_protected_variable<Task> RelatedData::TaskByID(const Poco::UInt64 id) const {
    auto tasks = Tasks();
    return { tasks_m_, modelByID<Task>(id, *tasks) };
}

const_protected_variable<Client> RelatedData::ClientByID(const Poco::UInt64 id) const {
    auto clients = Clients();
    return { clients_m_, modelByID(id, *clients) };
}

const_protected_variable<Project> RelatedData::ProjectByID(const Poco::UInt64 id) const {
    auto projects = Projects();
    return { projects_m_, modelByID(id, *projects) };
}

const_protected_variable<Tag> RelatedData::TagByID(const Poco::UInt64 id) const {
    auto tags = Tags();
    return { tags_m_, modelByID(id, *tags) };
}

const_protected_variable<Workspace> RelatedData::WorkspaceByID(const Poco::UInt64 id) const {
    auto workspaces = Workspaces();
    return { workspaces_m_, modelByID(id, *workspaces) };
}

const_protected_variable<TimeEntry> RelatedData::TimeEntryByID(const Poco::UInt64 id) const {
    auto timeEntries = TimeEntries();
    return { timeEntries_m_, modelByID(id, *timeEntries) };
}

const_protected_variable<TimeEntry> RelatedData::TimeEntryByGUID(const guid GUID) const {
    auto timeEntries = TimeEntries();
    return { timeEntries_m_, modelByGUID(GUID, *timeEntries) };
}

const_protected_variable<TimelineEvent> RelatedData::TimelineEventByGUID(const guid GUID) const {
    auto timelineEvents = TimelineEvents();
    return { timelineEvents_m_, modelByGUID(GUID, *timelineEvents) };
}

const_protected_variable<Tag> RelatedData::TagByGUID(const guid GUID) const {
    auto tags = Tags();
    return { tags_m_, modelByGUID(GUID, *tags) };
}

const_protected_variable<Project> RelatedData::ProjectByGUID(const guid GUID) const {
    auto projects = Projects();
    return { projects_m_, modelByGUID(GUID, *projects) };
}

const_protected_variable<Client> RelatedData::ClientByGUID(const guid GUID) const {
    auto clients = Clients();
    return { clients_m_, modelByGUID(GUID, *clients) };
}

template <typename T>
T *modelByGUID(const guid GUID, const std::set<T *> &list) {
    if (GUID.empty()) {
        return nullptr;
    }
    for (auto it : list) {
        T *model = it;
        if (model->GUID() == GUID) {
            return model;
        }
    }
    return nullptr;
}

template<typename T>
T *modelByID(const Poco::UInt64 id, const std::set<T *> &list) {
    if (!id) {
        return nullptr;
    }
    for (auto it : list) {
        T *model = it;
        if (model->ID() == id) {
            return model;
        }
    }
    return nullptr;
}

}   // namespace toggl
