// Copyright 2014 Toggl Desktop developers.

#include "./toggl_api_private.h"

#include <cstdlib>

#include "./formatter.h"
#include "./context.h"

TogglAutocompleteView *autocomplete_item_init(
    const toggl::AutocompleteItem item) {
    TogglAutocompleteView *result = new TogglAutocompleteView();
    result->Description = strdup(item.Description.c_str());
    result->Text = strdup(item.Text.c_str());
    result->ProjectAndTaskLabel = strdup(item.ProjectAndTaskLabel.c_str());
    result->TaskLabel = strdup(item.TaskLabel.c_str());
    result->ProjectLabel = strdup(item.ProjectLabel.c_str());
    result->ClientLabel = strdup(item.ClientLabel.c_str());
    result->ProjectColor = strdup(item.ProjectColor.c_str());
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
    poco_assert(p);

    TogglGenericView *result = view_item_init();
    result->ID = static_cast<unsigned int>(p->ID());
    result->GUID = strdup(p->GUID().c_str());
    result->Name = strdup(p->Name().c_str());
    return result;
}

TogglGenericView *tag_to_view_item(const std::string tag_name) {
    TogglGenericView *result = view_item_init();
    result->Name = strdup(tag_name.c_str());
    return result;
}

TogglGenericView *workspace_to_view_item(toggl::Workspace * const ws) {
    TogglGenericView *result = view_item_init();
    result->ID = static_cast<unsigned int>(ws->ID());
    result->Name = strdup(ws->Name().c_str());
    return result;
}

TogglGenericView *client_to_view_item(toggl::Client * const c) {
    TogglGenericView *result = view_item_init();
    result->ID = static_cast<unsigned int>(c->ID());
    result->WID = static_cast<unsigned int>(c->WID());
    result->GUID = strdup(c->GUID().c_str());
    result->Name = strdup(c->Name().c_str());
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

TogglTimeEntryView *time_entry_view_item_init(
    toggl::TimeEntry *te,
    const std::string project_and_task_label,
    const std::string task_label,
    const std::string project_label,
    const std::string client_label,
    const std::string color,
    const std::string date_duration,
    const std::string timeofday_format) {

    poco_check_ptr(te);

    TogglTimeEntryView *view_item = new TogglTimeEntryView();
    poco_check_ptr(view_item);

    view_item->DurationInSeconds = static_cast<int>(te->DurationInSeconds());
    view_item->Description = strdup(te->Description().c_str());
    view_item->GUID = strdup(te->GUID().c_str());
    view_item->WID = static_cast<unsigned int>(te->WID());
    view_item->TID = static_cast<unsigned int>(te->TID());
    view_item->PID = static_cast<unsigned int>(te->PID());
    view_item->Duration = strdup(te->DurationString().c_str());
    view_item->Started = static_cast<unsigned int>(te->Start());
    view_item->Ended = static_cast<unsigned int>(te->Stop());

    view_item->ProjectAndTaskLabel = strdup(project_and_task_label.c_str());
    view_item->TaskLabel = strdup(task_label.c_str());
    view_item->ProjectLabel = strdup(project_label.c_str());
    view_item->ClientLabel = strdup(client_label.c_str());
    view_item->Color = strdup(color.c_str());

    std::string start_time_string =
        toggl::Formatter::FormatTimeForTimeEntryEditor(te->Start(),
                timeofday_format);
    std::string end_time_string =
        toggl::Formatter::FormatTimeForTimeEntryEditor(te->Stop(),
                timeofday_format);

    view_item->StartTimeString = strdup(start_time_string.c_str());
    view_item->EndTimeString = strdup(end_time_string.c_str());

    view_item->DateDuration = strdup(date_duration.c_str());

    view_item->Billable = te->Billable();
    if (te->Tags().empty()) {
        view_item->Tags = 0;
    } else {
        view_item->Tags = strdup(te->Tags().c_str());
    }
    view_item->UpdatedAt = static_cast<unsigned int>(te->UpdatedAt());
    view_item->DateHeader = strdup(te->DateHeaderString().c_str());
    view_item->DurOnly = te->DurOnly();
    view_item->IsHeader = false;

    view_item->CanAddProjects = false;
    view_item->CanSeeBillable = false;
    view_item->DefaultWID = 0;

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
    view->OnTop = settings.on_top;
    view->Reminder = settings.reminder;
    view->UseIdleDetection = settings.use_idle_detection;

    view->UseProxy = use_proxy;

    view->ProxyHost = strdup(proxy.host.c_str());
    view->ProxyPort = proxy.port;
    view->ProxyUsername = strdup(proxy.username.c_str());
    view->ProxyPassword = strdup(proxy.password.c_str());

    return view;
}

void settings_view_item_clear(TogglSettingsView *view) {
    poco_check_ptr(view);

    free(view->ProxyHost);
    free(view->ProxyUsername);
    free(view->ProxyPassword);

    delete view;
}

_Bool testing_set_logged_in_user(
    void *context,
    const char *json) {
    poco_check_ptr(json);

    toggl::Context *app = reinterpret_cast<toggl::Context *>(context);
    return app->SetLoggedInUserFromJSON(std::string(json));
}
