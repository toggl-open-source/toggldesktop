// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_AUTOCOMPLETEVIEW_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_AUTOCOMPLETEVIEW_H_

#include <QObject>
#include <QVector>
#include <QDebug>

#include "toggl_api.h"

enum AutocompleteType {
    AC_TIME_ENTRY = 0,
    AC_TASK = 1,
    AC_PROJECT = 2,
    AC_HEADER = 11,
    AC_CLIENT = 12,
    AC_WORKSPACE = 13
};

class AutocompleteView : public QObject {
    Q_OBJECT

 public:
    explicit AutocompleteView(QObject *parent = 0);

    static QVector<AutocompleteView *> importAll(
        TogglAutocompleteView *first) {
        QVector<AutocompleteView *> result;
        TogglAutocompleteView *it = first;

        AutocompleteView *currentWorkspace = nullptr;
        AutocompleteView *currentLevel1Header = nullptr;
        AutocompleteView *currentLevel2Header = nullptr;
        AutocompleteView *currentLevel3Header = nullptr;
        AutocompleteView *previousProject = nullptr;
        uint64_t lastType = static_cast<uint64_t>(-1);

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
            view->WorkspaceID = it->WorkspaceID;
            view->WorkspaceName = QString(it->WorkspaceName);
            view->Type = it->Type;
            view->Billable = it->Billable;
            view->Tags = QString(it->Tags);

            if (!currentWorkspace || currentWorkspace->Description != view->WorkspaceName) {
                currentWorkspace = new AutocompleteView();
                currentWorkspace->Type = 13;
                currentWorkspace->Description = view->WorkspaceName;
                result.push_back(currentWorkspace);
                currentLevel1Header = nullptr;
                currentLevel2Header = nullptr;
                currentLevel3Header = nullptr;
                lastType = -1;
            }

            if (view->Type != lastType && lastType != 1 ) {
                if (view->Type == 1) {
                    if (previousProject)
                        currentLevel3Header = previousProject;
                }
                else {
                    currentLevel3Header = nullptr;
                    currentLevel2Header = nullptr;
                    currentLevel1Header = new AutocompleteView();
                    currentLevel1Header->Type = 11;
                    switch (view->Type) {
                    case 0:
                        currentLevel1Header->Description = "TIME ENTRIES";
                        break;
                    case 1:
                        // this won't happen, tasks are a special case
                        currentLevel1Header->Description = "TASKS";
                        break;
                    case 2:
                        currentLevel1Header->Description = "PROJECTS";
                        break;
                    default:
                        currentLevel1Header->Description = QString("UNHANDLED TYPE %1").arg(view->Type);
                        break;
                    }
                    result.push_back(currentLevel1Header);
                }
            }
            if (view->Type == 2 && (!currentLevel2Header || currentLevel2Header->ClientLabel != view->ClientLabel)) {
                currentLevel3Header = nullptr;
                currentLevel2Header = new AutocompleteView();
                currentLevel2Header->Type = 12;
                currentLevel2Header->ClientLabel = view->ClientLabel;
                if (view->ClientLabel.isEmpty())
                    currentLevel2Header->Description = "No client";
                else
                    currentLevel2Header->Description = view->ClientLabel;
                result.push_back(currentLevel2Header);
            }

            if (currentWorkspace && currentWorkspace->Description == view->WorkspaceName)
                currentWorkspace->_Children.append(view);

            if (currentLevel1Header)
                currentLevel1Header->_Children.append(view);
            if (currentLevel2Header)
                currentLevel2Header->_Children.append(view);
            if (currentLevel3Header)
                currentLevel3Header->_Children.append(view);

            lastType = view->Type;

            if (view->Type == 2) {
                previousProject = view;
            }

            result.push_back(view);
            it = static_cast<TogglAutocompleteView *>(it->Next);
        }

        return result;
    }

    QString Text {};
    QString Description {};
    QString ProjectAndTaskLabel {};
    QString ProjectLabel {};
    QString ClientLabel {};
    QString TaskLabel {};
    QString WorkspaceName {};
    QString ProjectColor {};
    uint64_t TaskID { 0 };
    uint64_t ProjectID { 0 };
    uint64_t ClientID { 0 };
    uint64_t WorkspaceID { 0 };
    uint64_t Type { 0 };
    bool Billable { false };
    QString Tags {};

    QList<AutocompleteView*> _Children;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_AUTOCOMPLETEVIEW_H_
