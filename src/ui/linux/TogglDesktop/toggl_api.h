#ifndef TOGGLAPI_H
#define TOGGLAPI_H

#include <QObject>
#include <QVector>
#include <QUrl>

#include <stdint.h>

#include "updateview.h"
#include "timeentryview.h"
#include "autocompleteview.h"
#include "genericview.h"
#include "settingsview.h"

class TogglApi : public QObject
{
    Q_OBJECT
public:
    explicit TogglApi(QObject *parent = 0);

    static TogglApi *instance;

    bool startEvents();

    void login(const QString email, const QString password);

    bool start(
        const QString description,
        const QString duration,
        const uint64_t task_id,
        const uint64_t project_id);

    bool stop();

    bool deleteTimeEntry(const QString guid);

    bool continueTimeEntry(const QString guid);

    void editTimeEntry(const QString guid,
        const QString focusedFieldName);

    void editRunningTimeEntry(
        const QString focusedFieldName);

    void viewTimeEntryList();

    bool setTimeEntryProject(
        const QString guid,
        const u_int64_t task_id,
        const u_int64_t project_id,
        const QString project_guid);

    bool setTimeEntryDescription(
        const QString guid,
        const QString value);

    bool setTimeEntryDuration(
        const QString guid,
        const QString value);

    bool setTimeEntryBillable(
        const QString guid,
        const bool billable);

    bool addProject(
        const QString time_entry_guid,
        const u_int64_t workspace_id,
        const u_int64_t client_id,
        const QString project_name,
        const bool is_private);

    static const QString formatDurationInSecondsHHMMSS(
        const int64_t duration);

signals:
    void displayApp(
        const bool open);

    void displayError(
        const QString errmsg,
        const bool user_error);

    void displayUpdate(
        const bool open,
        UpdateView *update);

    void displayOnlineState(
        const bool is_online);

    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void displayReminder(
        const QString title,
        const QString informative_text);

    void displayTimeEntryList(
        const bool open,
        QVector<TimeEntryView *> list);

    void displayTimeEntryEditor(
        const bool open,
        TimeEntryView *view,
        const QString focused_field_name);

    void displaySettings(
        const bool open,
        SettingsView *view);

    void displayStoppedTimerState();

    void displayRunningTimerState(
        TimeEntryView *view);

    void displayIdleNotification(
        const QString since,
        const QString duration,
        const uint64_t started);

    void displayClientSelect(
        QVector<GenericView *> list);

    void displayTags(
        QVector<GenericView *> list);

    void displayTimeEntryAutocomplete(
        QVector<AutocompleteView *> list);

    void displayProjectAutocomplete(
        QVector<AutocompleteView *> list);

    void displayWorkspaceSelect(
        QVector<GenericView *> list);

private:
    void *ctx;
};

#endif // TOGGLAPI_H
