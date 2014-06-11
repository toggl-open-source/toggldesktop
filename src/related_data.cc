// Copyright 2014 Toggl Desktop developers.

#include "./related_data.h"

#include <sstream>

#include "./autocomplete_item.h"
#include "./formatter.h"

namespace kopsik {

bool compareAutocompleteItems(
    AutocompleteItem a,
    AutocompleteItem b) {

    // Time entries first
    if (a.IsTimeEntry() && !b.IsTimeEntry()) {
        return true;
    }
    if (b.IsTimeEntry() && !(a.IsTimeEntry())) {
        return false;
    }

    // Then tasks
    if (a.IsTask() && !b.IsTask()) {
        return true;
    }
    if (b.IsTask() && !a.IsTask()) {
        return false;
    }

    // Then projects
    if (a.IsProject() && !b.IsProject()) {
        return true;
    }
    if (b.IsProject() && !a.IsProject()) {
        return false;
    }

    return (strcmp(a.Text.c_str(), b.Text.c_str()) < 0);
}

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
        }
        autocomplete_item.Type = kAutocompleteItemTE;
        list->push_back(autocomplete_item);
    }
}

// Add tasks, in format:
// Task. Project. Client
void RelatedData::taskAutocompleteItems(
    std::set<std::string> *unique_names,
    std::vector<AutocompleteItem> *list) {

    poco_check_ptr(list);

    for (std::vector<Task *>::const_iterator it =
        Tasks.begin();
            it != Tasks.end(); it++) {
        Task *t = *it;
        std::string project_label;
        std::string client_label;

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

        if (p) {
            project_label = p->Name();
        }

        Client *c = 0;
        if (p && p->CID()) {
            c = ClientByID(p->CID());
        }

        if (c) {
            client_label = c->Name();
        }

        std::string text = Formatter::JoinTaskNameReverse(t, p, c);
        if (text.empty()) {
            continue;
        }

        if (unique_names->find(text) != unique_names->end()) {
            continue;
        }
        unique_names->insert(text);

        AutocompleteItem autocomplete_item;
        autocomplete_item.Text = text;
        autocomplete_item.ProjectAndTaskLabel = text;
        autocomplete_item.ProjectLabel = project_label;
        autocomplete_item.ClientLabel = client_label;
        autocomplete_item.TaskID = t->ID();
        if (p) {
            autocomplete_item.ProjectColor = p->ColorCode();
            autocomplete_item.ProjectID = p->ID();
        }
        autocomplete_item.Type = kAutocompleteItemTask;
        list->push_back(autocomplete_item);
    }
}

// Add projects, in format:
// Project. Client
void RelatedData::projectAutocompleteItems(
    std::set<std::string> *unique_names,
    std::vector<AutocompleteItem> *list) {

    poco_check_ptr(list);

    for (std::vector<Project *>::const_iterator it =
        Projects.begin();
            it != Projects.end(); it++) {
        Project *p = *it;
        std::string project_label;
        std::string client_label;

        if (!p->Active()) {
            continue;
        }

        if (p) {
            project_label = p->Name();
        }

        Client *c = 0;
        if (p->CID()) {
            c = ClientByID(p->CID());
        }

        if (c) {
            client_label = c->Name();
        }

        std::string text = Formatter::JoinTaskName(0, p, c);
        if (text.empty()) {
            continue;
        }

        if (unique_names->find(text) != unique_names->end()) {
            continue;
        }
        unique_names->insert(text);

        AutocompleteItem autocomplete_item;
        autocomplete_item.Text = text;
        autocomplete_item.ProjectAndTaskLabel = text;
        autocomplete_item.ProjectLabel = project_label;
        autocomplete_item.ClientLabel = client_label;
        autocomplete_item.ProjectID = p->ID();
        autocomplete_item.ProjectColor = p->ColorCode();
        autocomplete_item.Type = kAutocompleteItemProject;
        list->push_back(autocomplete_item);
    }
}

std::vector<AutocompleteItem> RelatedData::AutocompleteItems(
    const bool including_time_entries) {
    std::vector<AutocompleteItem> result;

    std::set<std::string> unique_names;
    if (including_time_entries) {
        timeEntryAutocompleteItems(&unique_names, &result);
    }
    taskAutocompleteItems(&unique_names, &result);
    projectAutocompleteItems(&unique_names, &result);

    std::sort(result.begin(), result.end(), compareAutocompleteItems);
    return result;
}

}   // namespace kopsik
