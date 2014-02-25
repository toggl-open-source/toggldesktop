// Copyright 2014 Toggl Desktop developers.

#include "./kopsik_api_private.h"

KopsikModelChange *model_change_init() {
  KopsikModelChange *change = new KopsikModelChange();
  change->ModelType = 0;
  change->ChangeType = 0;
  change->ModelID = 0;
  change->GUID = 0;
  return change;
}

void model_change_clear(
    KopsikModelChange *change) {
  poco_assert(change);
  if (change->ModelType) {
    free(change->ModelType);
    change->ModelType = 0;
  }
  if (change->ChangeType) {
    free(change->ChangeType);
    change->ChangeType = 0;
  }
  if (change->GUID) {
    free(change->GUID);
    change->GUID = 0;
  }
  delete change;
}

void model_change_to_change_item(
    kopsik::ModelChange &in,
    KopsikModelChange &out) {

  poco_assert(in.ModelType() == "time_entry" ||
    in.ModelType() == "workspace" ||
    in.ModelType() == "client" ||
    in.ModelType() == "project" ||
    in.ModelType() == "user" ||
    in.ModelType() == "task" ||
    in.ModelType() == "tag");

  poco_assert(in.ChangeType() == "delete" ||
    in.ChangeType() == "insert" ||
    in.ChangeType() == "update");

  poco_assert(!in.GUID().empty() || in.ModelID() > 0);

  poco_assert(!out.ModelType);
  out.ModelType = strdup(in.ModelType().c_str());

  out.ModelID = (unsigned int)in.ModelID();

  poco_assert(!out.ChangeType);
  out.ChangeType = strdup(in.ChangeType().c_str());

  poco_assert(!out.GUID);
  out.GUID = strdup(in.GUID().c_str());
}

void time_entry_to_view_item(
    kopsik::TimeEntry *te,
    const std::string project_and_task_label,
    const std::string color_code,
    KopsikTimeEntryViewItem *view_item,
    const std::string dateDuration) {
  poco_assert(te);
  poco_assert(view_item);

  view_item->DurationInSeconds = static_cast<int>(te->DurationInSeconds());

  poco_assert(!view_item->Description);
  view_item->Description = strdup(te->Description().c_str());

  poco_assert(!view_item->GUID);
  view_item->GUID = strdup(te->GUID().c_str());

  view_item->TID = static_cast<unsigned int>(te->TID());
  view_item->PID = static_cast<unsigned int>(te->PID());

  poco_assert(!view_item->ProjectAndTaskLabel);
  view_item->ProjectAndTaskLabel = strdup(project_and_task_label.c_str());

  poco_assert(!view_item->Color);
  view_item->Color = strdup(color_code.c_str());

  poco_assert(!view_item->Duration);
  view_item->Duration = strdup(te->DurationString().c_str());

  view_item->Started = static_cast<unsigned int>(te->Start());
  view_item->Ended = static_cast<unsigned int>(te->Stop());
  if (te->Billable()) {
    view_item->Billable = 1;
  } else {
    view_item->Billable = 0;
  }

  poco_assert(!view_item->Tags);
  if (!te->Tags().empty()) {
    view_item->Tags = strdup(te->Tags().c_str());
  }

  view_item->UpdatedAt = static_cast<unsigned int>(te->UpdatedAt());

  poco_assert(!view_item->DateHeader);
  view_item->DateHeader = strdup(te->DateHeaderString().c_str());

  poco_assert(!view_item->DateDuration);
  if (!dateDuration.empty()) {
    view_item->DateDuration = strdup(dateDuration.c_str());
  }

  if (te->DurOnly()) {
    view_item->DurOnly = 1;
  } else {
    view_item->DurOnly = 0;
  }
}

KopsikAutocompleteItem *autocomplete_item_init() {
  KopsikAutocompleteItem *item = new KopsikAutocompleteItem();
  item->Text = 0;
  item->Description = 0;
  item->ProjectAndTaskLabel = 0;
  item->ProjectColor = 0;
  item->ProjectID = 0;
  item->TaskID = 0;
  item->Type = 0;
  item->Next = 0;
  return item;
}
