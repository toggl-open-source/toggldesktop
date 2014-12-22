// Copyright 2014 Toggl Desktop developers.

#include "./related_data.h"

#include <sstream>

#include "Poco/UTF8String.h"

#include "./autocomplete_item.h"
#include "./formatter.h"

namespace toggl {

// Add time entries, in format:
// Description - Task. Project. Client
void RelatedData::timeEntryAutocompleteItems(
    std::set<std::string> *unique_names,
    std::vector<AutocompleteItem> *list) {

    poco_check_ptr(list);

    for (std::vector<TimeEntry *>::const_iterator it =
        TimeEntries.begin();
            it != TimeEntries.end(); it++) {
        TimeEntry *te = *it;

        if (te->DeletedAt() || te->IsMarkedAsDeletedOnServer()
                || te->Description().empty()) {
            continue;
        }

        Task *t = 0;
        if (te->TID()) {
            t = TaskByID(te->TID());
        }

        Project *p = 0;
        if (t && t->PID()) {
            p = ProjectByID(t->PID());
        } else if (te->PID()) {
            p = ProjectByID(te->PID());
        }

        if (p && !p->Active()) {
            continue;
        }

        Client *c = 0;
        if (p && p->CID()) {
            c = ClientByID(p->CID());
        }

        std::string project_task_label =
            Formatter::JoinTaskNameReverse(t, p, c);

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

        AutocompleteItem autocomplete_item;
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
        }
        if (t) {
            autocomplete_item.TaskID = t->ID();
            autocomplete_item.TaskLabel = t->Name();
        }
        autocomplete_item.Type = kAutocompleteItemTE;
        list->push_back(autocomplete_item);
    }
}

// Add tasks, in format:
// Task. Project. Client
void RelatedData::taskAutocompleteItems(
    std::set<std::string> *unique_names,
    std::map<Poco::UInt64, std::string> *ws_names,
    std::vector<AutocompleteItem> *list) {

    poco_check_ptr(list);

    for (std::vector<Task *>::const_iterator it =
        Tasks.begin();
            it != Tasks.end(); it++) {
        Task *t = *it;

        if (t->IsMarkedAsDeletedOnServer()) {
            continue;
        }

        Project *p = 0;
        if (t->PID()) {
            p = ProjectByID(t->PID());
        }

        if (p && !p->Active()) {
            continue;
        }

        Client *c = 0;
        if (p && p->CID()) {
            c = ClientByID(p->CID());
        }

        std::string text = Formatter::JoinTaskNameReverse(t, p, c);
        if (text.empty()) {
            continue;
        }

        if (unique_names->find(text) != unique_names->end()) {
            continue;
        }
        unique_names->insert(text);

        std::string client_label("");
        if (c) {
            client_label = c->Name();
        }

        std::string project_label("");
        if (p) {
            project_label = p->Name();
        }

        AutocompleteItem autocomplete_item;
        autocomplete_item.Text = text;
        autocomplete_item.ProjectAndTaskLabel = text;
        autocomplete_item.TaskLabel = t->Name();
        autocomplete_item.ProjectLabel = project_label;
        autocomplete_item.ClientLabel = client_label;
        autocomplete_item.TaskID = t->ID();
        if (p) {
            autocomplete_item.ProjectColor = p->ColorCode();
            autocomplete_item.ProjectID = p->ID();
        }
        if (ws_names) {
            autocomplete_item.WorkspaceName = (*ws_names)[t->WID()];
        }
        autocomplete_item.Type = kAutocompleteItemTask;
        list->push_back(autocomplete_item);
    }
}

// Add projects, in format:
// Project. Client
void RelatedData::projectAutocompleteItems(
    std::set<std::string> *unique_names,
    std::map<Poco::UInt64, std::string> *ws_names,
    std::vector<AutocompleteItem> *list) {

    poco_check_ptr(list);

    for (std::vector<Project *>::const_iterator it =
        Projects.begin();
            it != Projects.end(); it++) {
        Project *p = *it;

        if (!p->Active()) {
            continue;
        }

        Client *c = 0;
        if (p->CID()) {
            c = ClientByID(p->CID());
        }

        std::string text = Formatter::JoinTaskName(0, p, c);
        if (text.empty()) {
            continue;
        }

        if (unique_names->find(text) != unique_names->end()) {
            continue;
        }
        unique_names->insert(text);

        std::string client_label("");
        if (c) {
            client_label = c->Name();
        }

        AutocompleteItem autocomplete_item;
        autocomplete_item.Text = text;
        autocomplete_item.ProjectAndTaskLabel = text;
        autocomplete_item.ProjectLabel = p->Name();
        autocomplete_item.ClientLabel = client_label;
        autocomplete_item.ProjectID = p->ID();
        autocomplete_item.ProjectColor = p->ColorCode();
        if (ws_names) {
            autocomplete_item.WorkspaceName = (*ws_names)[p->WID()];
        }
        autocomplete_item.Type = kAutocompleteItemProject;
        list->push_back(autocomplete_item);
    }
}

std::vector<AutocompleteItem> RelatedData::TimeEntryAutocompleteItems() {
    std::vector<AutocompleteItem> result;
    std::set<std::string> unique_names;
    timeEntryAutocompleteItems(&unique_names, &result);
    std::sort(result.begin(), result.end(), CompareAutocompleteItems);
    return result;
}

std::vector<AutocompleteItem> RelatedData::MinitimerAutocompleteItems() {
    std::vector<AutocompleteItem> result;
    std::set<std::string> unique_names;
    timeEntryAutocompleteItems(&unique_names, &result);
    taskAutocompleteItems(&unique_names, 0, &result);
    projectAutocompleteItems(&unique_names, 0, &result);
    std::sort(result.begin(), result.end(), CompareAutocompleteItems);
    return result;
}

std::vector<AutocompleteItem> RelatedData::ProjectAutocompleteItems() {
    std::vector<AutocompleteItem> result;
    std::set<std::string> unique_names;
    taskAutocompleteItems(&unique_names, 0, &result);
    projectAutocompleteItems(&unique_names, 0, &result);
    std::sort(result.begin(), result.end(), CompareAutocompleteItems);
    return result;
}

std::vector<AutocompleteItem> RelatedData::StructuredAutocompleteItems() {
    std::vector<AutocompleteItem> result;
    std::set<std::string> unique_names;
    std::map<Poco::UInt64, std::string> ws_names;
    workspaceAutocompleteItems(&unique_names, &ws_names, &result);
    projectAutocompleteItems(&unique_names, &ws_names, &result);
    taskAutocompleteItems(&unique_names, &ws_names, &result);
    std::sort(result.begin(), result.end(), CompareStructuredAutocompleteItems);
    return result;
}

void RelatedData::workspaceAutocompleteItems(
    std::set<std::string> *unique_names,
    std::map<Poco::UInt64, std::string> *ws_names,
    std::vector<AutocompleteItem> *list) {

    for (std::vector<Workspace *>::const_iterator it =
        Workspaces.begin();
            it != Workspaces.end(); it++) {
        Workspace *ws = *it;

        std::string ws_name = Poco::UTF8::toUpper(ws->Name());
        (*ws_names)[ws->ID()] = ws_name;

        AutocompleteItem autocomplete_item;
        autocomplete_item.Text = ws_name;
        autocomplete_item.WorkspaceName = ws_name;
        autocomplete_item.Type = kAutocompleteItemWorkspace;
        list->push_back(autocomplete_item);
    }
}

std::vector<std::string> RelatedData::TagList() const {
    std::vector<std::string> tags;
    std::set<std::string> unique_names;
    for (std::vector<Tag *>::const_iterator it =
        Tags.begin();
            it != Tags.end();
            it++) {
        Tag *tag = *it;
        if (unique_names.find(tag->Name()) != unique_names.end()) {
            continue;
        }
        unique_names.insert(tag->Name());
        tags.push_back(tag->Name());
    }
    std::sort(tags.rbegin(), tags.rend());
    return tags;
}

std::vector<Workspace *> RelatedData::WorkspaceList() const {
    std::vector<Workspace *> result = Workspaces;
    std::sort(result.rbegin(), result.rend(), CompareWorkspaceByName);
    return result;
}

std::vector<Client *> RelatedData::ClientList() const {
    std::vector<Client *> result = Clients;
    std::sort(result.rbegin(), result.rend(), CompareClientByName);
    return result;
}

void RelatedData::ProjectLabelAndColorCode(
    TimeEntry *te,
    std::string *workspace_name,
    std::string *project_and_task_label,
    std::string *task_label,
    std::string *project_label,
    std::string *client_label,
    std::string *color_code) const {

    poco_check_ptr(te);
    poco_check_ptr(workspace_name);
    poco_check_ptr(project_and_task_label);
    poco_check_ptr(task_label);
    poco_check_ptr(project_label);
    poco_check_ptr(client_label);
    poco_check_ptr(color_code);

    Workspace *ws = 0;
    if (te->WID()) {
        ws = WorkspaceByID(te->WID());
    }
    if (ws) {
        *workspace_name = ws->Name();
    }

    Task *t = 0;
    if (te->TID()) {
        t = TaskByID(te->TID());
    }
    if (t) {
        *task_label = t->Name();
    }

    Project *p = 0;
    if (t && t->PID()) {
        p = ProjectByID(t->PID());
    }
    if (!p && te->PID()) {
        p = ProjectByID(te->PID());
    }
    if (!p && !te->ProjectGUID().empty()) {
        p = ProjectByGUID(te->ProjectGUID());
    }

    Client *c = 0;
    if (p && p->CID()) {
        c = ClientByID(p->CID());
    }

    *project_and_task_label = Formatter::JoinTaskName(t, p, c);

    if (p) {
        *color_code = p->ColorCode();
        *project_label = p->Name();
    }

    if (c) {
        *client_label = c->Name();
    }
}

}   // namespace toggl
