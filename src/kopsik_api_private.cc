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

    poco_check_ptr(change);

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
    const kopsik::ModelChange in,
    KopsikModelChange *out) {

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

    out->ModelType = strdup(in.ModelType().c_str());
    out->ModelID = (unsigned int)in.ModelID();
    out->ChangeType = strdup(in.ChangeType().c_str());
    out->GUID = strdup(in.GUID().c_str());
}

void time_entry_to_view_item(
    kopsik::TimeEntry * const te,
    const std::string project_and_task_label,
    const std::string color_code,
    KopsikTimeEntryViewItem *view_item,
    const std::string dateDuration) {

    poco_check_ptr(te);
    poco_check_ptr(view_item);

    view_item->DurationInSeconds = static_cast<int>(te->DurationInSeconds());
    view_item->Description = strdup(te->Description().c_str());
    view_item->GUID = strdup(te->GUID().c_str());
    view_item->WID = static_cast<unsigned int>(te->WID());
    view_item->TID = static_cast<unsigned int>(te->TID());
    view_item->PID = static_cast<unsigned int>(te->PID());
    view_item->ProjectAndTaskLabel = strdup(project_and_task_label.c_str());
    view_item->Color = strdup(color_code.c_str());
    view_item->Duration = strdup(te->DurationString().c_str());
    view_item->Started = static_cast<unsigned int>(te->Start());
    view_item->Ended = static_cast<unsigned int>(te->Stop());
    if (te->Billable()) {
        view_item->Billable = true;
    } else {
        view_item->Billable = false;
    }
    if (!te->Tags().empty()) {
        view_item->Tags = strdup(te->Tags().c_str());
    }
    view_item->UpdatedAt = static_cast<unsigned int>(te->UpdatedAt());
    view_item->DateHeader = strdup(te->DateHeaderString().c_str());
    if (!dateDuration.empty()) {
        view_item->DateDuration = strdup(dateDuration.c_str());
    }
    if (te->DurOnly()) {
        view_item->DurOnly = true;
    } else {
        view_item->DurOnly = false;
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

KopsikViewItem *view_item_init() {
    KopsikViewItem *result = new KopsikViewItem();
    result->ID = 0;
    result->GUID = 0;
    result->Name = 0;
    return result;
}

KopsikViewItem *project_to_view_item(
    kopsik::Project * const p) {
    poco_assert(p);

    KopsikViewItem *result = view_item_init();
    result->ID = static_cast<unsigned int>(p->ID());
    result->GUID = strdup(p->GUID().c_str());
    result->Name = strdup(p->Name().c_str());
    return result;
}

KopsikViewItem *tag_to_view_item(
    const std::string tag_name) {
    KopsikViewItem *result = view_item_init();
    result->Name = strdup(tag_name.c_str());
    return result;
}

KopsikViewItem *workspace_to_view_item(
    kopsik::Workspace * const ws) {
    KopsikViewItem *result = view_item_init();
    result->ID = static_cast<unsigned int>(ws->ID());
    result->Name = strdup(ws->Name().c_str());
    return result;
}

KopsikViewItem *client_to_view_item(
    kopsik::Client * const c) {
    KopsikViewItem *result = view_item_init();
    result->ID = static_cast<unsigned int>(c->ID());
    result->GUID = strdup(c->GUID().c_str());
    result->Name = strdup(c->Name().c_str());
    return result;
}
