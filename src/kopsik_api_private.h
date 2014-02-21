// Copyright 2013 Tanel Lebedev

#ifndef SRC_KOPSIK_API_PRIVATE_H_
#define SRC_KOPSIK_API_PRIVATE_H_

#include <string>

#include "./kopsik_api.h"
#include "./context.h"

#define KOPSIK_API_FAILURE 1

void time_entry_to_view_item(
    kopsik::TimeEntry *te,
    kopsik::User *user,
    KopsikTimeEntryViewItem *view_item,
    const std::string dateDuration);

KopsikModelChange *model_change_init();

void model_change_to_change_item(
    kopsik::ModelChange &in,
    KopsikModelChange &out);

void model_change_clear(
    KopsikModelChange *change);

void autocomplete_item_clear(
    KopsikAutocompleteItem *item);

bool compareAutocompleteItems(
    KopsikAutocompleteItem *a,
    KopsikAutocompleteItem *b);

bool compareTags(
    kopsik::Tag *a,
    kopsik::Tag *b);

KopsikAutocompleteItem *autocomplete_item_init();

#endif  // SRC_KOPSIK_API_PRIVATE_H_
