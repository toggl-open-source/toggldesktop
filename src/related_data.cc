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

void RelatedData::Clear() {
    Workspaces()->clear();
    Clients()->clear();
    Projects()->clear();
    Tasks()->clear();
    Tags()->clear();
    TimeEntries()->clear();
    AutotrackerRules()->clear();
    TimelineEvents()->clear();
    ObmActions()->clear();
    ObmExperiments()->clear();
}

error RelatedData::DeleteAutotrackerRule(Poco::Int64 local_id) {
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
    const TimelineEvent &event) const {
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
    const std::string &lowercase_term) const {
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

locked<std::vector<TimelineEvent *> > RelatedData::VisibleTimelineEvents() {
    static std::vector<TimelineEvent *> result;
    result.clear();
    auto timelineEvents = TimelineEvents();
    for (auto it = timelineEvents->begin(); it != timelineEvents->end(); it++) {
        TimelineEvent *event = *it;
        if (event && event->VisibleToUser()) {
            result.push_back(event);
        }
    }
    return TimelineEvents.make_locked(&result);
}

locked<std::vector<TimeEntry *> > RelatedData::VisibleTimeEntries() {
    static std::vector<TimeEntry *> result;
    result.clear();
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
    return TimeEntries.make_locked(&result);
}

Poco::Int64 RelatedData::TotalDurationForDate(const TimeEntry &match) const {
    std::string date_header = Formatter::FormatDateHeader(match.Start());
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

locked<const TimeEntry> RelatedData::LatestTimeEntry() const {
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

    return TimeEntries.make_locked(latest);
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

        locked<const Task> t;
        if (te->TID()) {
            t = Tasks.findByID(te->TID());
        }

        locked<const Project> p;
        if (t && t->PID()) {
            p = Projects.findByID(t->PID());
        } else if (te->PID()) {
            p = Projects.findByID(te->PID());
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

        locked<const Project> p;
        if (t->PID()) {
            p = Projects.findByID(t->PID());
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

        locked<const Client> c;
        if (p)
            c = clientByProject(*p);

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

void RelatedData::TagList(std::vector<std::string> *tag_names,
    Poco::UInt64 wid) const {

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

locked<std::vector<Workspace*>> RelatedData::WorkspaceList() {
    static std::vector<Workspace*> result;
    result.clear();

    auto workspaces = Workspaces();
    for (auto it = workspaces->begin(); it != workspaces->end(); it++) {
        Workspace *ws = *it;
        if (!ws->Admin() && ws->OnlyAdminsMayCreateProjects()) {
            continue;
        }
        result.push_back(ws);
    }

    // OVERHAUL_TODO
    //std::sort(result->rbegin(), result->rend(), CompareWorkspaces.findByName);

    return Workspaces.make_locked(&result);
}

locked<std::vector<Client *> > RelatedData::ClientList() {
    static std::vector<Client*> result;
    result.clear();

    auto clients = Clients();
    for (auto i : *clients) {
        result.push_back(i);
    }
    // OVERHAUL_TODO
    //std::sort(result->rbegin(), result->rend(), CompareClients.findByName);

    return Clients.make_locked(&result);
}

void RelatedData::ProjectLabelAndColorCode(const TimeEntry &te,
    view::TimeEntry *view) const {

    poco_check_ptr(view);

    locked<const Workspace> ws;
    if (te.WID()) {
        ws = Workspaces.findByID(te.WID());
    }
    if (ws) {
        view->WorkspaceName = ws->Name();
    }

    locked<const Task> t;
    if (te.TID()) {
        t = Tasks.findByID(te.TID());
    }
    if (t) {
        view->TaskLabel = t->Name();
    }

    locked<const Project> p;
    if (t && t->PID()) {
        p = Projects.findByID(t->PID());
    }
    if (!p && te.PID()) {
        p = Projects.findByID(te.PID());
    }
    if (!p && !te.ProjectGUID().empty()) {
        p = Projects.findByGUID(te.ProjectGUID());
    }

    locked<const Client> c;
    if (p)
        c = clientByProject(*p);

    view->ProjectAndTaskLabel = Formatter::JoinTaskName(t.data(), p.data());

    if (p) {
        view->Color = p->ColorCode();
        view->ProjectLabel = p->Name();
    }

    if (c) {
        view->ClientLabel = c->Name();
    }
}

locked<Client> RelatedData::clientByProject(const Project &p) {
    locked<Client> c;
    if (p.CID()) {
        c = Clients.findByID(p.CID());
    }
    if (!c && !p.ClientGUID().empty()) {
        c = Clients.findByGUID(p.ClientGUID());
    }
    return c;
}

locked<const Client> RelatedData::clientByProject(const Project &p) const {
    locked<const Client> c;
    if (p.CID()) {
        c = Clients.findByID(p.CID());
    }
    if (!c && !p.ClientGUID().empty()) {
        c = Clients.findByGUID(p.ClientGUID());
    }
    return c;
}

}   // namespace toggl
