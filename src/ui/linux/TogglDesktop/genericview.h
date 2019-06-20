// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_GENERICVIEW_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_GENERICVIEW_H_

#include <QObject>
#include <QVector>

#include "./gui.h"
#include "./toggl_api.h"

class GenericView : public QObject, public toggl::view::Generic {
    Q_OBJECT

 public:
    explicit GenericView(QObject *parent = nullptr, const toggl::view::Generic *view = nullptr);

    static QVector<GenericView *> importAll(const TogglGenericView *first) {
        QVector<GenericView *> result;
        const TogglGenericView *it = first;
        while (it) {
            const toggl::view::Generic *v = reinterpret_cast<const toggl::view::Generic*>(it);
            auto sv = new GenericView(nullptr, v);
            result.push_back(sv);
            it = TogglGenericView_Next(it);
        }
        return result;
    }
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_GENERICVIEW_H_
