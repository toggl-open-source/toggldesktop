// Copyright 2014 Toggl Desktop developers.

#include "./settingsview.h"

SettingsView::SettingsView(QObject *parent, const toggl::view::Settings *view)
    : QObject(parent)
    , toggl::view::Settings(*view) {
}
