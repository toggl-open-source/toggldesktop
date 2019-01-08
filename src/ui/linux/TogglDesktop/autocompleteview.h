// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_AUTOCOMPLETEVIEW_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_AUTOCOMPLETEVIEW_H_

#include <QObject>
#include <QVector>
#include <QDebug>

#include "./toggl_api.h"

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
            }

            if (view->Type != lastType) {
                currentLevel1Header = new AutocompleteView();
                currentLevel1Header->Type = 11;
                switch (view->Type) {
                case 0:
                    currentLevel1Header->Description = "TIME ENTRIES";
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

            if (view->Type == 0 && currentWorkspace && currentWorkspace->Description == view->WorkspaceName)
                currentWorkspace->Text.append(" " + view->Text + " " + view->Description);

            if (currentLevel1Header)
                currentLevel1Header->Text.append(" " + view->Text + " " + view->Description + " " + view->ProjectLabel + " " + view->ClientLabel + " " + view->TaskLabel + " " + view->ProjectAndTaskLabel);
            if (currentLevel2Header)
                currentLevel2Header->Text.append(" " + view->Text + " " + view->Description + " " + view->ProjectLabel + " " + view->ClientLabel + " " + view->TaskLabel + " " + view->ProjectAndTaskLabel);
            if (currentLevel3Header)
                currentLevel3Header->Text.append(" " + view->Text + " " + view->Description + " " + view->ProjectLabel + " " + view->ClientLabel + " " + view->TaskLabel + " " + view->ProjectAndTaskLabel);

            lastType = view->Type;

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
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_AUTOCOMPLETEVIEW_H_
