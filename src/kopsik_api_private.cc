// Copyright 2013 Tanel Lebedev

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
    kopsik::User *user,
    KopsikTimeEntryViewItem *view_item,
    const std::string dateDuration) {
  poco_assert(te);
  poco_assert(user);
  poco_assert(view_item);

  view_item->DurationInSeconds = te->DurationInSeconds();

  poco_assert(!view_item->Description);
  view_item->Description = strdup(te->Description().c_str());

  poco_assert(!view_item->GUID);
  view_item->GUID = strdup(te->GUID().c_str());

  kopsik::Task *t = 0;
  if (te->TID()) {
    t = user->GetTaskByID(te->TID());
  }
  view_item->TID = te->TID();

  kopsik::Project *p = 0;
  if (t) {
    p = user->GetProjectByID(t->PID());
  } else if (te->PID()) {
    p = user->GetProjectByID(te->PID());
  }
  if (p) {
    view_item->PID = p->ID();
  } else {
    view_item->PID = 0;
  }

  kopsik::Client *c = 0;
  if (p && p->CID()) {
    c = user->GetClientByID(p->CID());
  }

  poco_assert(!view_item->ProjectAndTaskLabel);
  view_item->ProjectAndTaskLabel = strdup(user->JoinTaskName(t, p, c).c_str());

  poco_assert(!view_item->Color);
  if (p) {
    view_item->Color = strdup(p->ColorCode().c_str());
  }

  poco_assert(!view_item->Duration);
  view_item->Duration = strdup(te->DurationString().c_str());

  view_item->Started = te->Start();
  view_item->Ended = te->Stop();
  if (te->Billable()) {
    view_item->Billable = 1;
  } else {
    view_item->Billable = 0;
  }

  poco_assert(!view_item->Tags);
  if (!te->Tags().empty()) {
    view_item->Tags = strdup(te->Tags().c_str());
  }

  view_item->UpdatedAt = te->UpdatedAt();

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
  return item;
}

void autocomplete_item_clear(
    KopsikAutocompleteItem *item) {
  if (item->Text) {
    free(item->Text);
    item->Text = 0;
  }
  if (item->ProjectAndTaskLabel) {
    free(item->ProjectAndTaskLabel);
    item->ProjectAndTaskLabel = 0;
  }
  if (item->Description) {
    free(item->Description);
    item->Description = 0;
  }
  if (item->ProjectColor) {
    free(item->ProjectColor);
    item->ProjectColor = 0;
  }
  delete item;
}

bool isTimeEntry(KopsikAutocompleteItem *n) {
  return KOPSIK_AUTOCOMPLETE_TE == n->Type;
}

bool isTask(KopsikAutocompleteItem *n) {
  return KOPSIK_AUTOCOMPLETE_TASK == n->Type;
}

bool isProject(KopsikAutocompleteItem *n) {
  return KOPSIK_AUTOCOMPLETE_PROJECT == n->Type;
}

bool compareAutocompleteItems(
    KopsikAutocompleteItem *a,
    KopsikAutocompleteItem *b) {
  // Time entries first
  if (isTimeEntry(a) && !isTimeEntry(b)) {
    return true;
  }
  if (isTimeEntry(b) && !(isTimeEntry(a))) {
    return false;
  }

  // Then tasks
  if (isTask(a) && !isTask(b)) {
    return true;
  }
  if (isTask(b) && !isTask(a)) {
    return false;
  }

  // Then projects
  if (isProject(a) && !isProject(b)) {
    return true;
  }
  if (isProject(b) && !isProject(a)) {
    return false;
  }

  return (strcmp(a->Text, b->Text) < 0);
}

bool compareTags(
    kopsik::Tag *a,
    kopsik::Tag *b) {
  return (strcmp(a->Name().c_str(), b->Name().c_str()) > 0);
}
