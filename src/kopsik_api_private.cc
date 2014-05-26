// Copyright 2014 Toggl Desktop developers.

#include "./kopsik_api_private.h"

#include <cstdlib>

#include "./formatter.h"
#include "./context.h"

KopsikAutocompleteItem *autocomplete_item_init(
    const kopsik::AutocompleteItem item) {
    KopsikAutocompleteItem *result = new KopsikAutocompleteItem();
    result->Description = strdup(item.Description.c_str());
    result->Text = strdup(item.Text.c_str());
    result->ProjectAndTaskLabel = strdup(item.ProjectAndTaskLabel.c_str());
    result->ProjectColor = strdup(item.ProjectColor.c_str());
    result->ProjectID = static_cast<unsigned int>(item.ProjectID);
    result->TaskID = static_cast<unsigned int>(item.TaskID);
    result->Type = static_cast<unsigned int>(item.Type);
    result->Next = 0;
    return result;
}

KopsikViewItem *view_item_init() {
    KopsikViewItem *result = new KopsikViewItem();
    result->ID = 0;
    result->WID = 0;
    result->GUID = 0;
    result->Name = 0;
    return result;
}

KopsikViewItem *project_to_view_item(kopsik::Project * const p) {
    poco_assert(p);

    KopsikViewItem *result = view_item_init();
    result->ID = static_cast<unsigned int>(p->ID());
    result->GUID = strdup(p->GUID().c_str());
    result->Name = strdup(p->Name().c_str());
    return result;
}

KopsikViewItem *tag_to_view_item(const std::string tag_name) {
    KopsikViewItem *result = view_item_init();
    result->Name = strdup(tag_name.c_str());
    return result;
}

KopsikViewItem *workspace_to_view_item(kopsik::Workspace * const ws) {
    KopsikViewItem *result = view_item_init();
    result->ID = static_cast<unsigned int>(ws->ID());
    result->Name = strdup(ws->Name().c_str());
    return result;
}

KopsikViewItem *client_to_view_item(kopsik::Client * const c) {
    KopsikViewItem *result = view_item_init();
    result->ID = static_cast<unsigned int>(c->ID());
    result->WID = static_cast<unsigned int>(c->WID());
    result->GUID = strdup(c->GUID().c_str());
    result->Name = strdup(c->Name().c_str());
    return result;
}

void view_item_clear(KopsikViewItem *item) {
    if (!item) {
        return;
    }

    free(item->Name);
    item->Name = 0;

    free(item->GUID);
    item->GUID = 0;

    if (item->Next) {
        KopsikViewItem *next = reinterpret_cast<KopsikViewItem *>(item->Next);
        view_item_clear(next);
    }

    delete item;
    item = 0;
}

void autocomplete_item_clear(KopsikAutocompleteItem *item) {
    if (!item) {
        return;
    }

    free(item->Text);
    item->Text = 0;

    free(item->ProjectAndTaskLabel);
    item->ProjectAndTaskLabel = 0;

    free(item->Description);
    item->Description = 0;

    free(item->ProjectColor);
    item->ProjectColor = 0;

    if (item->Next) {
        KopsikAutocompleteItem *next =
            reinterpret_cast<KopsikAutocompleteItem *>(item->Next);
        autocomplete_item_clear(next);
        item->Next = 0;
    }

    delete item;
}

KopsikTimeEntryViewItem *time_entry_view_item_init(
    kopsik::TimeEntry *te,
    const std::string project_and_task_label,
    const std::string color,
    const std::string date_duration,
    const std::string timeofday_format) {

    poco_check_ptr(te);

    KopsikTimeEntryViewItem *view_item = new KopsikTimeEntryViewItem();
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
    view_item->Color = strdup(color.c_str());

    std::string start_time_string =
        kopsik::Formatter::FormatTimeForTimeEntryEditor(te->Start(),
                timeofday_format);
    std::string end_time_string =
        kopsik::Formatter::FormatTimeForTimeEntryEditor(te->Stop(),
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

    view_item->Next = 0;

    return view_item;
}

void time_entry_view_item_clear(
    KopsikTimeEntryViewItem *item) {
    if (!item) {
        return;
    }
    free(item->Description);
    item->Description = 0;

    free(item->ProjectAndTaskLabel);
    item->ProjectAndTaskLabel = 0;

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
        KopsikTimeEntryViewItem *next =
            reinterpret_cast<KopsikTimeEntryViewItem *>(item->Next);
        time_entry_view_item_clear(next);
        item->Next = 0;
    }

    delete item;
}

KopsikSettingsViewItem *settings_view_item_init(
    const _Bool record_timeline,
    const kopsik::Settings settings,
    const _Bool use_proxy,
    const kopsik::Proxy proxy) {
    KopsikSettingsViewItem *view = new KopsikSettingsViewItem();

    view->RecordTimeline = record_timeline;

    view->DockIcon = settings.dock_icon;
    view->MenubarTimer = settings.menubar_timer;
    view->OnTop = settings.on_top;
    view->Reminder = settings.reminder;
    view->IgnoreCert = settings.ignore_cert;
    view->UseIdleDetection = settings.use_idle_detection;

    view->UseProxy = use_proxy;

    view->ProxyHost = strdup(proxy.host.c_str());
    view->ProxyPort = proxy.port;
    view->ProxyUsername = strdup(proxy.username.c_str());
    view->ProxyPassword = strdup(proxy.password.c_str());

    return view;
}

void settings_view_item_clear(KopsikSettingsViewItem *view) {
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

    kopsik::Context *app = reinterpret_cast<kopsik::Context *>(context);
    return app->SetLoggedInUserFromJSON(std::string(json));
}
