// Copyright 2014 Toggl Desktop developers.

#include "related_data.h"

#include <algorithm>
#include <sstream>

#include <Poco/UTF8String.h>

#include "autotracker.h"
#include "formatter.h"
#include "client.h"
#include "gui.h"
#include "obm_action.h"
#include "project.h"
#include "tag.h"
#include "task.h"
#include "time_entry.h"
#include "workspace.h"

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
    Workspaces.clear();
    Clients.clear();
    Projects.clear();
    Tasks.clear();
    Tags.clear();
    TimeEntries.clear();
    AutotrackerRules.clear();
    TimelineEvents.clear();
    ObmActions.clear();
    ObmExperiments.clear();
}

error RelatedData::DeleteAutotrackerRule(const Poco::Int64 local_id) {
    if (!local_id) {
        return error("cannot delete rule without an ID");
    }
    for (auto rule : AutotrackerRules) {
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

locked<AutotrackerRule> RelatedData::FindAutotrackerRule(locked<TimelineEvent> &event) {
    for (auto rule : AutotrackerRules) {
        if (rule->Matches(event)) {
            return rule;
        }
    }
    return {};
}

bool RelatedData::HasMatchingAutotrackerRule(
    const std::string &lowercase_term) const {
    for (auto rule : AutotrackerRules) {
        if (rule->Term() == lowercase_term) {
            return true;
        }
    }
    return false;
}

Poco::Int64 RelatedData::NumberOfUnsyncedTimeEntries() const {
    Poco::Int64 count(0);

    for (auto te : TimeEntries) {
        if (te->NeedsPush()) {
            count++;
        }
    }

    return count;
}

std::vector<locked<TimelineEvent>> RelatedData::VisibleTimelineEvents() {
    std::vector<locked<TimelineEvent>> result;
    for (auto event : TimelineEvents) {
        if (event && event->VisibleToUser()) {
            result.push_back(std::move(event));
        }
    }
    return result;
}

std::vector<locked<TimeEntry>> RelatedData::VisibleTimeEntries() {
    std::vector<locked<TimeEntry>> result;
    for (auto te : TimeEntries) {
        if (te->GUID().empty()) {
            continue;
        }
        if (te->DeletedAt() > 0) {
            continue;
        }
        result.push_back(std::move(te));
    }
    return result;
}

Poco::Int64 RelatedData::TotalDurationForDate(locked<TimeEntry> &match) {
    std::string date_header = Formatter::FormatDateHeader(match->Start());
    Poco::Int64 duration(0);
    for (auto te : TimeEntries) {
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

locked<TimeEntry> RelatedData::LatestTimeEntry() {
    locked<TimeEntry> latest;
    std::string pomodoro_decription("Pomodoro Break");
    std::string pomodoro_tag("pomodoro-break");

    // Find the time entry that was stopped most recently
    for (auto te : TimeEntries) {
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
            // beware of the std::move, the te variable is invalid after using it
            latest = std::move(te);
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

    for (auto te : TimeEntries) {
        if (te->DeletedAt() || te->IsMarkedAsDeletedOnServer()
                || te->Description().empty()) {
            continue;
        }

        locked<const Task> t;
        if (te->TID()) {
            t = Tasks.byID(te->TID());
        }

        locked<const Project> p;
        if (t && t->PID()) {
            p = Projects.byID(t->PID());
        } else if (te->PID()) {
            p = Projects.byID(te->PID());
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
    std::map<Poco::UInt64, std::vector<view::Autocomplete> > *items) const {

    poco_check_ptr(list);

    for (auto t : Tasks) {
        if (t->IsMarkedAsDeletedOnServer()) {
            continue;
        }

        if (!t->Active()) {
            continue;
        }

        locked<const Project> p;
        if (t->PID()) {
            p = Projects.byID(t->PID());
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
    std::map<Poco::UInt64, std::vector<view::Autocomplete> > *task_items) const {

    poco_check_ptr(list);

    for (auto p : Projects) {
        if (!p->Active()) {
            continue;
        }

        locked<const Client> c = clientByProject(p);

        locked<const Task> emptyTask;
        std::string text = Formatter::JoinTaskName(emptyTask, p);
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
                        it != (*task_items)[autocomplete_item.ProjectID].end(); ++it) {
                    view::Autocomplete ac = *it;
                    (*items)[autocomplete_item.WorkspaceName].push_back(ac);
                }
            }
        } else {
            list->push_back(autocomplete_item);
            if (task_items) {
                for (std::vector<view::Autocomplete>::const_iterator it =
                    (*task_items)[autocomplete_item.ProjectID].begin();
                        it != (*task_items)[autocomplete_item.ProjectID].end(); ++it) {
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
    std::map<Poco::UInt64, std::vector<view::Autocomplete> > task_items;

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
    std::map<Poco::UInt64, std::vector<view::Autocomplete> > task_items;
    workspaceAutocompleteItems(&unique_names, &ws_names, result);
    taskAutocompleteItems(&unique_names, &ws_names, result, &task_items);
    projectAutocompleteItems(&unique_names, &ws_names, result, nullptr, &task_items);
}

void RelatedData::workspaceAutocompleteItems(
    std::set<std::string> *,
    std::map<Poco::UInt64, std::string> *ws_names,
    std::vector<view::Autocomplete> *) const {

    // remember workspaces that have projects
    std::set<Poco::UInt64> ws_ids_with_projects;
    for (auto p : Projects) {
        if (p->Active()) {
            ws_ids_with_projects.insert(p->WID());
        }
    }

    for (auto ws : Workspaces) {
        if (ws_ids_with_projects.find(ws->ID()) == ws_ids_with_projects.end()) {
            continue;
        }

        std::string ws_name = ws->Name();
        (*ws_names)[ws->ID()] = ws_name;
    }
}

void RelatedData::TagList(
    std::vector<std::string> *tags,
    const Poco::UInt64 wid) const {

    poco_check_ptr(tags);

    std::set<std::string> unique_names;

    for (auto tag : Tags) {
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

std::vector<locked<Workspace>> RelatedData::WorkspaceList() {
    std::vector<locked<Workspace>> result;
    for (auto ws : Workspaces) {
        if (!ws->Admin() && ws->OnlyAdminsMayCreateProjects()) {
            continue;
        }
        result.push_back(std::move(ws));
    }
    std::sort(result.rbegin(), result.rend(), CompareWorkspaceByName);
    return result;
}

std::vector<locked<Client>> RelatedData::ClientList() {
    std::vector<locked<Client>> result;
    for (auto i : Clients) {
        result.push_back(std::move(i));
    }
    // TODO maybe later, now it doesn't fullfill all requirements
    //std::copy(Clients.begin(), Clients.end(), result.begin());
    std::sort(result.rbegin(), result.rend(), CompareClientByName);
    return result;
}

void RelatedData::ProjectLabelAndColorCode(locked<TimeEntry> &te,
    view::TimeEntry *view) const {

    poco_check_ptr(te);
    poco_check_ptr(view);

    locked<const Workspace> ws;
    if (te->WID()) {
        ws = Workspaces.byID(te->WID());
    }
    if (ws) {
        view->WorkspaceName = ws->Name();
    }

    locked<const Task> t;
    if (te->TID()) {
        t = Tasks.byID(te->TID());
    }
    if (t) {
        view->TaskLabel = t->Name();
    }

    locked<const Project> p;
    if (t && t->PID()) {
        p = Projects.byID(t->PID());
    }
    if (!p && te->PID()) {
        p = Projects.byID(te->PID());
    }
    if (!p && !te->ProjectGUID().empty()) {
        p = Projects.byGUID(te->ProjectGUID());
    }

    locked<const Client> c;

    view->ProjectAndTaskLabel = Formatter::JoinTaskName(t, p);

    if (p) {
        view->Color = p->ColorCode();
        view->ProjectLabel = p->Name();
    }

    if (c) {
        view->ClientLabel = c->Name();
    }
}

locked<Client> RelatedData::clientByProject(locked<Project> &p) {
    locked<Client> c;
    if (p && p->CID()) {
        c = Clients.byID(p->CID());
    }
    if (!c && p && !p->ClientGUID().empty()) {
        c = Clients.byGUID(p->ClientGUID());
    }
    return c;
}

locked<const Client> RelatedData::clientByProject(locked<const Project> &p) const {
    locked<const Client> c;
    if (p && p->CID()) {
        c = Clients.byID(p->CID());
    }
    if (!c && p && !p->ClientGUID().empty()) {
        c = Clients.byGUID(p->ClientGUID());
    }
    return c;

}

}   // namespace toggl
