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

void RelatedData::forEachTimeEntries(std::function<void(TimeEntry *)> f) {
    auto TEs = GetTimeEntries();
    std::for_each(TEs.second->begin(), TEs.second->end(), f);
}

void RelatedData::pushBackTimeEntry(TimeEntry *timeEntry) {
    auto TEs = GetTimeEntries();
    TEs.second->push_back(timeEntry);
}

protected_vector<Workspace *> RelatedData::GetWorkspaces() {
    return std::make_pair(std::unique_lock<std::mutex>(_WorkspacesMutex), &_Workspaces);
}

protected_vector<Client *> RelatedData::GetClients() {
    return std::make_pair(std::unique_lock<std::mutex>(_ClientsMutex), &_Clients);
}

protected_vector<Project *> RelatedData::GetProjects() {
    return std::make_pair(std::unique_lock<std::mutex>(_ProjectsMutex), &_Projects);
}

protected_vector<Task *> RelatedData::GetTasks() {
    return std::make_pair(std::unique_lock<std::mutex>(_TasksMutex), &_Tasks);
}

protected_vector<Tag *> RelatedData::GetTags() {
    return std::make_pair(std::unique_lock<std::mutex>(_TagsMutex), &_Tags);
}

protected_vector<TimeEntry *> RelatedData::GetTimeEntries() {
    return std::make_pair(std::unique_lock<std::mutex>(_TimeEntriesMutex), &_TimeEntries);
}

protected_vector<AutotrackerRule *> RelatedData::GetAutotrackerRules() {
    return std::make_pair(std::unique_lock<std::mutex>(_AutotrackerRulesMutex), &_AutotrackerRules);
}

protected_vector<TimelineEvent *> RelatedData::GetTimelineEvents() {
    return std::make_pair(std::unique_lock<std::mutex>(_TimelineEventsMutex), &_TimelineEvents);
}

protected_vector<ObmAction *> RelatedData::GetObmActions() {
    return std::make_pair(std::unique_lock<std::mutex>(_ObmActionsMutex), &_ObmActions);
}

protected_vector<ObmExperiment *> RelatedData::GetObmExperiments() {
    return std::make_pair(std::unique_lock<std::mutex>(_ObmExperimentsMutex), &_ObmExperiments);
}

protected_const_vector<Workspace *> RelatedData::GetWorkspaces() const {
    return std::make_pair(std::unique_lock<std::mutex>(_WorkspacesMutex), &_Workspaces);
}

protected_const_vector<Client *> RelatedData::GetClients() const {
    return std::make_pair(std::unique_lock<std::mutex>(_ClientsMutex), &_Clients);
}

protected_const_vector<Project *> RelatedData::GetProjects() const {
    return std::make_pair(std::unique_lock<std::mutex>(_ProjectsMutex), &_Projects);
}

protected_const_vector<Task *> RelatedData::GetTasks() const {
    return std::make_pair(std::unique_lock<std::mutex>(_TasksMutex), &_Tasks);
}

protected_const_vector<Tag *> RelatedData::GetTags() const {
    return std::make_pair(std::unique_lock<std::mutex>(_TagsMutex), &_Tags);
}

protected_const_vector<TimeEntry *> RelatedData::GetTimeEntries() const {
    return std::make_pair(std::unique_lock<std::mutex>(_TimeEntriesMutex), &_TimeEntries);
}

protected_const_vector<AutotrackerRule *> RelatedData::GetAutotrackerRules() const {
    return std::make_pair(std::unique_lock<std::mutex>(_AutotrackerRulesMutex), &_AutotrackerRules);
}

protected_const_vector<TimelineEvent *> RelatedData::GetTimelineEvents() const {
    return std::make_pair(std::unique_lock<std::mutex>(_TimelineEventsMutex), &_TimelineEvents);
}

protected_const_vector<ObmAction *> RelatedData::GetObmActions() const {
    return std::make_pair(std::unique_lock<std::mutex>(_ObmActionsMutex), &_ObmActions);
}

protected_const_vector<ObmExperiment *> RelatedData::GetObmExperiments() const {
    return std::make_pair(std::unique_lock<std::mutex>(_ObmExperimentsMutex), &_ObmExperiments);
}

void RelatedData::Clear() {
    auto Workspaces = GetWorkspaces();
    clearList(Workspaces.second);
    Workspaces.first.unlock();

    auto Clients = GetClients();
    clearList(Clients.second);
    Clients.first.unlock();

    auto Projects = GetProjects();
    clearList(Projects.second);
    Projects.first.unlock();

    auto Tasks = GetTasks();
    clearList(Tasks.second);
    Tasks.first.unlock();

    auto Tags = GetTags();
    clearList(Tags.second);
    Tags.first.unlock();

    auto TimeEntries = GetTimeEntries();
    clearList(TimeEntries.second);
    TimeEntries.first.unlock();

    auto AutotrackerRules = GetAutotrackerRules();
    clearList(AutotrackerRules.second);
    AutotrackerRules.first.unlock();

    auto TimelineEvents = GetTimelineEvents();
    clearList(TimelineEvents.second);
    TimelineEvents.first.unlock();

    auto ObmActions = GetObmActions();
    clearList(ObmActions.second);
    ObmActions.first.unlock();

    auto ObmExperiments = GetObmExperiments();
    clearList(ObmExperiments.second);
    ObmExperiments.first.unlock();
}

error RelatedData::DeleteAutotrackerRule(const Poco::Int64 local_id) {
    if (!local_id) {
        return error("cannot delete rule without an ID");
    }
    auto ARs = GetAutotrackerRules();
    for (std::vector<AutotrackerRule *>::iterator it = ARs.second->begin(); it != ARs.second->end(); it++) {
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
    auto ARs = GetAutotrackerRules();
    for (std::vector<AutotrackerRule *>::const_iterator it = ARs.second->begin(); it != ARs.second->end(); it++) {
        AutotrackerRule *rule = *it;
        if (rule->Matches(event)) {
            return rule;
        }
    }
    return nullptr;
}

bool RelatedData::HasMatchingAutotrackerRule(
    const std::string lowercase_term) const {
    auto ARs = GetAutotrackerRules();
    for (std::vector<AutotrackerRule *>::const_iterator it = ARs.second->begin(); it != ARs.second->end(); it++) {
        AutotrackerRule *rule = *it;
        if (rule->Term() == lowercase_term) {
            return true;
        }
    }
    return false;
}

Poco::Int64 RelatedData::NumberOfUnsyncedTimeEntries() const {
    Poco::Int64 count(0);
    auto TEs = GetTimeEntries();
    for (std::vector<TimeEntry *>::const_iterator it = TEs.second->begin(); it != TEs.second->end(); it++) {
        TimeEntry *te = *it;
        if (te->NeedsPush()) {
            count++;
        }
    }

    return count;
}

std::vector<TimelineEvent *> RelatedData::VisibleTimelineEvents() const {
    std::vector<TimelineEvent *> result;
    auto TLEs = GetTimelineEvents();
    for (std::vector<TimelineEvent *>::const_iterator i = TLEs.second->begin(); i != TLEs.second->end(); ++i) {
        TimelineEvent *event = *i;
        if (event && event->VisibleToUser()) {
            result.push_back(event);
        }
    }
    return result;
}

std::vector<TimeEntry *> RelatedData::VisibleTimeEntries() const {
    std::vector<TimeEntry *> result;
    auto TEs = GetTimeEntries();
    for (std::vector<TimeEntry *>::const_iterator it = TEs.second->begin(); it != TEs.second->end(); it++) {
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
    auto TEs = GetTimeEntries();
    for (std::vector<TimeEntry *>::const_iterator it = TEs.second->begin(); it != TEs.second->end(); it++) {
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
    auto TEs = GetTimeEntries();
    // Find the time entry that was stopped most recently
    for (std::vector<TimeEntry *>::const_iterator it = TEs.second->begin(); it != TEs.second->end(); it++) {
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

    auto TEs = GetTimeEntries();
    for (std::vector<TimeEntry *>::const_iterator it = TEs.second->begin(); it != TEs.second->end(); it++) {
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

        std::string project_task_label =
            Formatter::JoinTaskName(t, p);

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

    auto Ts = GetTasks();
    for (std::vector<Task *>::const_iterator it = Ts.second->begin(); it != Ts.second->end(); it++) {
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

        Project *p = nullptr;
        if (t->PID()) {
            p = ProjectByID(t->PID());
        }

        if (p && !p->Active()) {
            continue;
        }

        std::string text = Formatter::JoinTaskName(t, p);
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

    auto Ps = GetProjects();
    for (std::vector<Project *>::const_iterator it = Ps.second->begin(); it != Ps.second->end(); it++) {
        Project *p = *it;

        if (!p->Active()) {
            continue;
        }

        Client *c = clientByProject(p);

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

    auto Ps = GetProjects();
    // remember workspaces that have projects
    std::set<Poco::UInt64> ws_ids_with_projects;
    for (std::vector<Project *>::const_iterator it = Ps.second->begin(); it != Ps.second->end(); it++) {
        Project *p = *it;

        if (p->Active()) {
            ws_ids_with_projects.insert(p->WID());
        }
    }
    Ps.first.unlock();

    auto WSs = GetWorkspaces();
    for (std::vector<Workspace *>::const_iterator it = WSs.second->begin(); it != WSs.second->end(); it++) {
        Workspace *ws = *it;

        if (ws_ids_with_projects.find(ws->ID()) == ws_ids_with_projects.end()) {
            continue;
        }

        std::string ws_name = Poco::UTF8::toUpper(ws->Name());
        (*ws_names)[ws->ID()] = ws_name;
    }
    WSs.first.unlock();
}

void RelatedData::TagList(
    std::vector<std::string> *tags,
    const Poco::UInt64 wid) const {

    poco_check_ptr(tags);

    std::set<std::string> unique_names;

    auto Ts = GetTags();
    for (std::vector<Tag *>::const_iterator it = Ts.second->begin(); it != Ts.second->end(); it++) {
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
    Ts.first.unlock();

    std::sort(tags->rbegin(), tags->rend());
}

void RelatedData::WorkspaceList(std::vector<Workspace *> *result) const {

    poco_check_ptr(result);

    auto WSs = GetWorkspaces();
    for (std::vector<Workspace *>::const_iterator it = WSs.second->begin(); it != WSs.second->end(); it++) {
        Workspace *ws = *it;
        if (!ws->Admin() && ws->OnlyAdminsMayCreateProjects()) {
            continue;
        }
        result->push_back(ws);
    }
    std::sort(result->rbegin(), result->rend(), CompareWorkspaceByName);
}

void RelatedData::ClientList(std::vector<Client *> *result) const {

    poco_check_ptr(result);
    auto Cs = GetClients();
    *result = *Cs.second;
    Cs.first.unlock();

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

    view->ProjectAndTaskLabel = Formatter::JoinTaskName(t, p);

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
    auto Tasks = GetTasks();
    return modelByID<Task>(id, Tasks.second);
}

Client *RelatedData::ClientByID(const Poco::UInt64 id) const {
    auto Clients = GetClients();
    return modelByID(id, Clients.second);
}

Project *RelatedData::ProjectByID(const Poco::UInt64 id) const {
    auto Projects = GetProjects();
    return modelByID(id, Projects.second);
}

Tag *RelatedData::TagByID(const Poco::UInt64 id) const {
    auto Tags = GetTags();
    return modelByID(id, Tags.second);
}

Workspace *RelatedData::WorkspaceByID(const Poco::UInt64 id) const {
    auto Workspaces = GetWorkspaces();
    return modelByID(id, Workspaces.second);
}

TimeEntry *RelatedData::TimeEntryByID(const Poco::UInt64 id) const {
    auto TimeEntries = GetTimeEntries();
    return modelByID(id, TimeEntries.second);
}

TimeEntry *RelatedData::TimeEntryByGUID(const guid GUID) const {
    auto TimeEntries = GetTimeEntries();
    return modelByGUID(GUID, TimeEntries.second);
}

TimelineEvent *RelatedData::TimelineEventByGUID(const guid GUID) const {
    auto TimelineEvents = GetTimelineEvents();
    return modelByGUID(GUID, TimelineEvents.second);
}

Tag *RelatedData::TagByGUID(const guid GUID) const {
    auto Tags = GetTags();
    return modelByGUID(GUID, Tags.second);
}

Project *RelatedData::ProjectByGUID(const guid GUID) const {
    auto Projects = GetProjects();
    return modelByGUID(GUID, Projects.second);
}

Client *RelatedData::ClientByGUID(const guid GUID) const {
    auto Clients = GetClients();
    return modelByGUID(GUID, Clients.second);
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
