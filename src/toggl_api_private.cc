// Copyright 2014 Toggl Desktop developers.

#include "../src/toggl_api_private.h"

#include <cstdlib>

#include "./client.h"
#include "./context.h"
#include "./formatter.h"
#include "./project.h"
#include "./time_entry.h"
#include "./workspace.h"

#include "Poco/Logger.h"
#include "Poco/UnicodeConverter.h"

TogglAutocompleteView *autocomplete_item_init(
    const toggl::AutocompleteItem item) {
    TogglAutocompleteView *result = new TogglAutocompleteView();
    result->Description = copy_string(item.Description);
    result->Text = copy_string(item.Text);
    result->ProjectAndTaskLabel = copy_string(item.ProjectAndTaskLabel);
    result->TaskLabel = copy_string(item.TaskLabel);
    result->ProjectLabel = copy_string(item.ProjectLabel);
    result->ClientLabel = copy_string(item.ClientLabel);
    result->ProjectColor = copy_string(item.ProjectColor);
    result->ProjectID = static_cast<unsigned int>(item.ProjectID);
    result->TaskID = static_cast<unsigned int>(item.TaskID);
    result->Type = static_cast<unsigned int>(item.Type);
    result->Next = 0;
    return result;
}

TogglGenericView *view_item_init() {
    TogglGenericView *result = new TogglGenericView();
    result->ID = 0;
    result->WID = 0;
    result->GUID = 0;
    result->Name = 0;
    return result;
}

TogglGenericView *project_to_view_item(toggl::Project * const p) {
    if (!p) {
        logger().error("Cannot render 0 project");
        return 0;
    }

    TogglGenericView *result = view_item_init();
    result->ID = static_cast<unsigned int>(p->ID());
    result->GUID = copy_string(p->GUID());
    result->Name = copy_string(p->Name());
    return result;
}

TogglGenericView *tag_to_view_item(const std::string tag_name) {
    TogglGenericView *result = view_item_init();
    result->Name = copy_string(tag_name);
    return result;
}

TogglGenericView *workspace_to_view_item(toggl::Workspace * const ws) {
    TogglGenericView *result = view_item_init();
    result->ID = static_cast<unsigned int>(ws->ID());
    result->Name = copy_string(ws->Name());
    return result;
}

TogglGenericView *client_to_view_item(toggl::Client * const c) {
    TogglGenericView *result = view_item_init();
    result->ID = static_cast<unsigned int>(c->ID());
    result->WID = static_cast<unsigned int>(c->WID());
    result->GUID = copy_string(c->GUID());
    result->Name = copy_string(c->Name());
    return result;
}

void view_item_clear(TogglGenericView *item) {
    if (!item) {
        return;
    }

    free(item->Name);
    item->Name = 0;

    free(item->GUID);
    item->GUID = 0;

    if (item->Next) {
        TogglGenericView *next =
            reinterpret_cast<TogglGenericView *>(item->Next);
        view_item_clear(next);
    }

    delete item;
    item = 0;
}

void autocomplete_item_clear(TogglAutocompleteView *item) {
    if (!item) {
        return;
    }

    free(item->Text);
    item->Text = 0;

    free(item->ProjectAndTaskLabel);
    item->ProjectAndTaskLabel = 0;

    free(item->TaskLabel);
    item->TaskLabel = 0;

    free(item->ProjectLabel);
    item->ProjectLabel = 0;

    free(item->ClientLabel);
    item->ClientLabel = 0;

    free(item->Description);
    item->Description = 0;

    free(item->ProjectColor);
    item->ProjectColor = 0;

    if (item->Next) {
        TogglAutocompleteView *next =
            reinterpret_cast<TogglAutocompleteView *>(item->Next);
        autocomplete_item_clear(next);
        item->Next = 0;
    }

    delete item;
}

std::string to_string(const char_t *s) {
#ifdef _WIN32
    std::wstring ws(s);
    std::string res("");
    Poco::UnicodeConverter::toUTF8(ws, res);
    return res;
#else
    return std::string(s);
#endif
}

char_t *copy_string(const std::string s) {
#ifdef _WIN32
    std::wstring ws;
    Poco::UnicodeConverter::toUTF16(s, ws);
    return wcsdup(ws.c_str());
#else
    return strdup(s.c_str());
#endif
}

int compare_string(const char_t *s1, const char_t *s2) {
#ifdef _WIN32
    return wcscmp(s1, s2);
#else
    return strcmp(s1, s2);
#endif
}

TogglTimeEntryView *time_entry_view_item_init(
    toggl::TimeEntry *te,
    const std::string workspace_name,
    const std::string project_and_task_label,
    const std::string task_label,
    const std::string project_label,
    const std::string client_label,
    const std::string color,
    const std::string date_duration,
    const bool time_in_timer_format) {

    poco_check_ptr(te);

    TogglTimeEntryView *view_item = new TogglTimeEntryView();
    poco_check_ptr(view_item);

    view_item->DurationInSeconds = static_cast<int>(te->DurationInSeconds());
    view_item->Description = copy_string(te->Description());
    view_item->GUID = copy_string(te->GUID());
    view_item->WID = static_cast<unsigned int>(te->WID());
    view_item->TID = static_cast<unsigned int>(te->TID());
    view_item->PID = static_cast<unsigned int>(te->PID());
    if (time_in_timer_format) {
        view_item->Duration =
            toggl_format_tracking_time_duration(te->DurationInSeconds());
    } else {
        view_item->Duration = copy_string(toggl::Formatter::FormatDuration(
            te->DurationInSeconds(), toggl::Formatter::DurationFormat));
    }
    view_item->Started = static_cast<unsigned int>(te->Start());
    view_item->Ended = static_cast<unsigned int>(te->Stop());

    view_item->WorkspaceName = copy_string(workspace_name);
    view_item->ProjectAndTaskLabel = copy_string(project_and_task_label);
    view_item->TaskLabel = copy_string(task_label);
    view_item->ProjectLabel = copy_string(project_label);
    view_item->ClientLabel = copy_string(client_label);
    view_item->Color = copy_string(color);

    std::string start_time_string =
        toggl::Formatter::FormatTimeForTimeEntryEditor(te->Start());
    std::string end_time_string =
        toggl::Formatter::FormatTimeForTimeEntryEditor(te->Stop());

    view_item->StartTimeString = copy_string(start_time_string);
    view_item->EndTimeString = copy_string(end_time_string);

    view_item->DateDuration = copy_string(date_duration);

    view_item->Billable = te->Billable();
    if (te->Tags().empty()) {
        view_item->Tags = 0;
    } else {
        view_item->Tags = copy_string(te->Tags().c_str());
    }
    view_item->UpdatedAt = static_cast<unsigned int>(te->UpdatedAt());
    view_item->DateHeader = copy_string(te->DateHeaderString());
    view_item->DurOnly = te->DurOnly();
    view_item->IsHeader = false;

    view_item->CanAddProjects = false;
    view_item->CanSeeBillable = false;
    view_item->DefaultWID = 0;

    if (te->ValidationError() != toggl::noError) {
        view_item->Error = copy_string(te->ValidationError());
    } else {
        view_item->Error = 0;
    }

    view_item->Next = 0;

    return view_item;
}

void time_entry_view_item_clear(
    TogglTimeEntryView *item) {
    if (!item) {
        return;
    }
    free(item->Description);
    item->Description = 0;

    free(item->WorkspaceName);
    item->WorkspaceName = 0;

    free(item->ProjectAndTaskLabel);
    item->ProjectAndTaskLabel = 0;

    free(item->TaskLabel);
    item->TaskLabel = 0;

    free(item->ProjectLabel);
    item->ProjectLabel = 0;

    free(item->ClientLabel);
    item->ClientLabel = 0;

    free(item->Duration);
    item->Duration = 0;

    free(item->Color);
    item->Color = 0;

    free(item->GUID);
    item->GUID = 0;

    free(item->Tags);
    item->Tags = 0;

    free(item->DateHeader);
    item->DateHeader = 0;

    free(item->DateDuration);
    item->DateDuration = 0;

    free(item->StartTimeString);
    item->StartTimeString = 0;

    free(item->EndTimeString);
    item->EndTimeString = 0;

    if (item->Error) {
        free(item->Error);
        item->Error = 0;
    }

    if (item->Next) {
        TogglTimeEntryView *next =
            reinterpret_cast<TogglTimeEntryView *>(item->Next);
        time_entry_view_item_clear(next);
        item->Next = 0;
    }

    delete item;
}

TogglSettingsView *settings_view_item_init(
    const _Bool record_timeline,
    const toggl::Settings settings,
    const _Bool use_proxy,
    const toggl::Proxy proxy) {
    TogglSettingsView *view = new TogglSettingsView();

    view->RecordTimeline = record_timeline;

    view->DockIcon = settings.dock_icon;
    view->MenubarTimer = settings.menubar_timer;
    view->MenubarProject = settings.menubar_project;
    view->OnTop = settings.on_top;
    view->Reminder = settings.reminder;
    view->UseIdleDetection = settings.use_idle_detection;
    view->IdleMinutes = settings.idle_minutes;
    view->FocusOnShortcut = settings.focus_on_shortcut;
    view->ReminderMinutes = settings.reminder_minutes;
    view->ManualMode = settings.manual_mode;
    view->AutodetectProxy = settings.autodetect_proxy;

    view->UseProxy = use_proxy;

    view->ProxyHost = copy_string(proxy.Host());
    view->ProxyPort = proxy.Port();
    view->ProxyUsername = copy_string(proxy.Username());
    view->ProxyPassword = copy_string(proxy.Password());

    return view;
}

void settings_view_item_clear(TogglSettingsView *view) {
    poco_check_ptr(view);

    free(view->ProxyHost);
    free(view->ProxyUsername);
    free(view->ProxyPassword);

    delete view;
}

TogglAutocompleteView *autocomplete_list_init(
    std::vector<toggl::AutocompleteItem> *items) {
    TogglAutocompleteView *first = 0;
    for (std::vector<toggl::AutocompleteItem>::const_reverse_iterator it =
        items->rbegin(); it != items->rend(); it++) {
        TogglAutocompleteView *item = autocomplete_item_init(*it);
        item->Next = first;
        first = item;
    }
    return first;
}

Poco::Logger &logger() {
    return Poco::Logger::get("toggl_api");
}

toggl::Context *app(void *context) {
    poco_check_ptr(context);

    return reinterpret_cast<toggl::Context *>(context);
}
