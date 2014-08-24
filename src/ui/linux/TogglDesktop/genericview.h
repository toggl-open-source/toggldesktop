// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_GENERICVIEW_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_GENERICVIEW_H_

#include <QObject>
#include <QVector>

#include "./toggl_api.h"

class GenericView : public QObject {
    Q_OBJECT

 public:
    explicit GenericView(QObject *parent = 0);

    static QVector<GenericView *> importAll(TogglGenericView *first) {
        QVector<GenericView *> result;
        TogglGenericView *it = first;
        while (it) {
            GenericView *view = new GenericView();
            view->ID = it->ID;
            view->WID = it->WID;
            view->GUID = QString(it->GUID);
            view->Name = QString(it->Name);
            result.push_back(view);
            it = static_cast<TogglGenericView *>(it->Next);
        }
        return result;
    }

    uint64_t ID;
    uint64_t WID;
    QString GUID;
    QString Name;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_GENERICVIEW_H_
