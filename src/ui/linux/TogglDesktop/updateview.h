// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_UPDATEVIEW_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_UPDATEVIEW_H_

#include <QObject>

#include "./toggl_api.h"

class UpdateView : public QObject {
    Q_OBJECT
 public:
    explicit UpdateView(QObject *parent = 0);

    static UpdateView *importOne(TogglUpdateView *view) {
        UpdateView *result = new UpdateView();
        result->UpdateChannel = QString(view->UpdateChannel);
        result->IsChecking = view->IsChecking;
        result->IsUpdateAvailable = view->IsUpdateAvailable;
        result->URL = QString(view->URL);
        result->Version = QString(view->Version);
        return result;
    }

    QString UpdateChannel;
    bool IsChecking;
    bool IsUpdateAvailable;
    QString URL;
    QString Version;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_UPDATEVIEW_H_
