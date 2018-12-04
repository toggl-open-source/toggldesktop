// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_AUTOCOMPLETEVIEW_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_AUTOCOMPLETEVIEW_H_

#include <QObject>
#include <QVector>

#include "./toggl_api.h"

class AutocompleteView : public QObject {
    Q_OBJECT

 public:
    explicit AutocompleteView(QObject *parent = 0);

    static QVector<AutocompleteView *> importAll(
        TogglAutocompleteView *first) {
        QVector<AutocompleteView *> result;
        TogglAutocompleteView *it = first;
        while (it) {
            AutocompleteView *view = new AutocompleteView();
            view->Text = QString(it->Text);
            view->Description = QString(it->Description);
            view->ProjectAndTaskLabel = QString(it->ProjectAndTaskLabel);
            view->ProjectLabel = QString(it->ProjectLabel);
            view->ClientLabel = QString(it->ClientLabel);
            view->ProjectColor = QString(it->ProjectColor);
            view->ClientID = it->ClientID;
            view->TaskID = it->TaskID;
            view->TaskLabel = it->TaskLabel;
            view->ProjectID = it->ProjectID;
            view->ProjectLabel = QString(it->ProjectLabel);
            view->WorkspaceID = it->WorkspaceID;
            view->WorkspaceName = QString(it->WorkspaceName);
            view->Type = it->Type;
            view->Billable = it->Billable;
            view->Tags = QString(it->Tags);
            result.push_back(view);
            it = static_cast<TogglAutocompleteView *>(it->Next);
        }
        return result;
    }

    QString Text;
    QString Description;
    QString ProjectAndTaskLabel;
    QString ProjectLabel;
    QString ClientLabel;
    QString TaskLabel;
    QString WorkspaceName;
    QString ProjectColor;
    uint64_t TaskID;
    uint64_t ProjectID;
    uint64_t ClientID;
    uint64_t WorkspaceID;
    uint64_t Type;
    bool Billable;
    QString Tags;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_AUTOCOMPLETEVIEW_H_
