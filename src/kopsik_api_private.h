// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_KOPSIK_API_PRIVATE_H_
#define SRC_KOPSIK_API_PRIVATE_H_

#include <string>

#include "./kopsik_api.h"
#include "./project.h"
#include "./client.h"
#include "./autocomplete_item.h"
#include "./workspace.h"
#include "./time_entry.h"
#include "./settings.h"
#include "./proxy.h"

KopsikViewItem *view_item_init();

KopsikViewItem *project_to_view_item(
    kopsik::Project * const);

KopsikViewItem *tag_to_view_item(
    const std::string tag_name);

KopsikViewItem *workspace_to_view_item(
    kopsik::Workspace * const);

KopsikViewItem *client_to_view_item(
    kopsik::Client * const);

KopsikAutocompleteItem *autocomplete_item_init(
    const kopsik::AutocompleteItem item);

void view_item_clear(KopsikViewItem *item);

void autocomplete_item_clear(KopsikAutocompleteItem *item);

KopsikTimeEntryViewItem *time_entry_view_item_init(
    kopsik::TimeEntry *te,
    const std::string project_and_task_label,
    const std::string color,
    const std::string date_duration);

void time_entry_view_item_clear(KopsikTimeEntryViewItem *item);

KopsikSettingsViewItem *settings_view_item_init(
    const _Bool record_timeline,
    const kopsik::Settings settings,
    const _Bool use_proxy,
    const kopsik::Proxy proxy);

void settings_view_item_clear(KopsikSettingsViewItem *view);

// For testing only
_Bool testing_set_logged_in_user(
    void *context,
    const char *json);

#endif  // SRC_KOPSIK_API_PRIVATE_H_
