// Copyright 2014 Toggl Desktop developers.

#include "./autocompleteview.h"

AutocompleteView::AutocompleteView(QObject *parent) : QObject(parent) {
}

AutocompleteView::AutocompleteView(QObject *parent, const toggl::view::Autocomplete *view)
    : QObject(parent)
    , toggl::view::Autocomplete(*view)
{

}
