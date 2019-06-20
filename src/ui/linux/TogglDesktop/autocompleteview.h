// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_AUTOCOMPLETEVIEW_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_AUTOCOMPLETEVIEW_H_

#include <QObject>
#include <QVector>
#include <QDebug>

#include "gui.h"
#include "./toggl_api.h"

enum AutocompleteType {
    AC_TIME_ENTRY = 0,
    AC_TASK = 1,
    AC_PROJECT = 2,
    AC_HEADER = 11,
    AC_CLIENT = 12,
    AC_WORKSPACE = 13
};

class AutocompleteView : public QObject, public toggl::view::Autocomplete {
    Q_OBJECT

 public:
    explicit AutocompleteView(QObject *parent = 0);
    explicit AutocompleteView(QObject *parent, const toggl::view::Autocomplete *view);

    static QVector<AutocompleteView *> importAll(
        const TogglAutocompleteView *first) {
        QVector<AutocompleteView *> result;

        const TogglAutocompleteView *it = first;

        AutocompleteView *currentWorkspace = nullptr;
        AutocompleteView *currentLevel1Header = nullptr;
        AutocompleteView *currentLevel2Header = nullptr;
        AutocompleteView *currentLevel3Header = nullptr;
        AutocompleteView *previousProject = nullptr;
        uint64_t lastType = static_cast<uint64_t>(-1);

        while (it) {
            const toggl::view::Autocomplete *v = reinterpret_cast<const toggl::view::Autocomplete*>(it);
            AutocompleteView *view = new AutocompleteView(nullptr, v);

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
                        currentLevel1Header->Description = ("UNHANDLED TYPE " + view->Type);
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
                if (view->ClientLabel.empty())
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
            //it = static_cast<TogglAutocompleteView *>(it->Next);
        }

        return result;
    }

    QList<AutocompleteView*> _Children;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_AUTOCOMPLETEVIEW_H_
