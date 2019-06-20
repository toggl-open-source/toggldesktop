// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_SETTINGSVIEW_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_SETTINGSVIEW_H_

#include <QObject>
#include <QTime>

#include "./gui.h"
#include "./toggl_api.h"

class SettingsView : public QObject, public toggl::view::Settings {
    Q_OBJECT

 public:
    explicit SettingsView(QObject *parent = nullptr, const toggl::view::Settings *view = nullptr);

    static SettingsView *importOne(const TogglSettingsView *view) {
        const toggl::view::Settings *v = reinterpret_cast<const toggl::view::Settings*>(view);
        return new SettingsView(nullptr, v);
    }
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_SETTINGSVIEW_H_
