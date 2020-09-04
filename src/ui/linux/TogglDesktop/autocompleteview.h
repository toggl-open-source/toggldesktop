// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_AUTOCOMPLETEVIEW_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_AUTOCOMPLETEVIEW_H_

#include <QObject>
#include <QVector>
#include <QDebug>

#include "toggl.h"

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

    enum AutocompleteType {
        AC_TIME_ENTRY = 0,
        AC_TASK = 1,
        AC_PROJECT = 2,
        AC_HEADER = 11,
        AC_CLIENT = 12,
        AC_WORKSPACE = 13
    };
    Q_ENUMS(AutocompleteType)

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
            view->Text = toQString(it->Text);
            view->Description = toQString(it->Description);
            view->ProjectAndTaskLabel = toQString(it->ProjectAndTaskLabel);
            view->ProjectLabel = toQString(it->ProjectLabel);
            view->ClientLabel = toQString(it->ClientLabel);
            view->ProjectColor = toQString(it->ProjectColor);
            view->ClientID = it->ClientID;
            view->TaskID = it->TaskID;
            view->TaskLabel = toQString(it->TaskLabel);
            view->ProjectID = it->ProjectID;
            view->WorkspaceID = it->WorkspaceID;
            view->WorkspaceName = toQString(it->WorkspaceName);
            view->Type = AutocompleteType(it->Type);
            view->Billable = it->Billable;
            view->Tags = toQString(it->Tags);

            if (!currentWorkspace || currentWorkspace->Description != view->WorkspaceName) {
                currentWorkspace = new AutocompleteView();
                currentWorkspace->Type = AC_WORKSPACE;
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
                    currentLevel1Header->Type = AC_HEADER;
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
                currentLevel2Header->Type = AC_CLIENT;
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

    PROPERTY(QString, Text)
    PROPERTY(QString, Description)
    PROPERTY(QString, ProjectAndTaskLabel)
    PROPERTY(QString, ProjectLabel)
    PROPERTY(QString, ClientLabel)
    PROPERTY(QString, TaskLabel)
    PROPERTY(QString, WorkspaceName)
    PROPERTY(QString, ProjectColor)
    PROPERTY(uint64_t, TaskID)
    PROPERTY(uint64_t, ProjectID)
    PROPERTY(uint64_t, ClientID)
    PROPERTY(uint64_t, WorkspaceID)
    PROPERTY(AutocompleteType, Type)
    PROPERTY(bool, Billable)
    PROPERTY(QString, Tags)
    QList<AutocompleteView*> _Children;

private:
    mutable QMutex propertyMutex_;

};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_AUTOCOMPLETEVIEW_H_
