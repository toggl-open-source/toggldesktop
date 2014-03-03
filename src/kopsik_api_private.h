// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_KOPSIK_API_PRIVATE_H_
#define SRC_KOPSIK_API_PRIVATE_H_

#include <string>

#include "./kopsik_api.h"
#include "./context.h"

#define KOPSIK_API_FAILURE 1

KopsikViewItem *view_item_init();

void time_entry_to_view_item(
  kopsik::TimeEntry * const,
  const std::string project_and_task_label,
  const std::string color_code,
  KopsikTimeEntryViewItem *view_item,
  const std::string dateDuration);

KopsikViewItem *project_to_view_item(
  kopsik::Project * const);

KopsikViewItem *tag_to_view_item(
  const std::string tag_name);

KopsikViewItem *workspace_to_view_item(
  kopsik::Workspace * const);

KopsikViewItem *client_to_view_item(
  kopsik::Client * const);

KopsikModelChange *model_change_init();

void model_change_to_change_item(
  const kopsik::ModelChange in,
  KopsikModelChange *out);

void model_change_clear(
  KopsikModelChange *change);

void autocomplete_item_clear(
  KopsikAutocompleteItem *item);

KopsikAutocompleteItem *autocomplete_item_init();

#endif  // SRC_KOPSIK_API_PRIVATE_H_
