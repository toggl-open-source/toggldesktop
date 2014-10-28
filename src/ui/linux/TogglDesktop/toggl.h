// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_TOGGL_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_TOGGL_H_

#include <QObject>
#include <QVector>
#include <QUrl>

#include <stdint.h>

#include "./updateview.h"
#include "./timeentryview.h"
#include "./autocompleteview.h"
#include "./genericview.h"
#include "./settingsview.h"

class TogglApi : public QObject {
    Q_OBJECT

 public:
    explicit TogglApi(QObject *parent = 0);

    static TogglApi *instance;

    static QString Project;
    static QString Duration;
    static QString Description;

    bool shutdown;

    bool startEvents();

    void setEnvironment(const QString environment);

    void login(const QString email, const QString password);

    void googleLogin(const QString accessToken);

    bool start(
        const QString description,
        const QString duration,
        const uint64_t task_id,
        const uint64_t project_id);

    bool stop();

    bool deleteTimeEntry(const QString guid);

    bool continueTimeEntry(const QString guid);

    bool continueLatestTimeEntry();

    void openInBrowser();

    void sync();

    void about();

    bool clearCache();

    void getSupport();

    void logout();

    void editPreferences();

    void editTimeEntry(const QString guid,
                       const QString focusedFieldName);

    void editRunningTimeEntry(
        const QString focusedFieldName);

    void viewTimeEntryList();

    void setIdleSeconds(u_int64_t idleSeconds);

    bool setTimeEntryProject(
        const QString guid,
        const uint64_t task_id,
        const uint64_t project_id,
        const QString project_guid);

    bool setTimeEntryDescription(
        const QString guid,
        const QString value);

    bool setTimeEntryTags(
        const QString guid,
        const QString tags);

    bool setTimeEntryDuration(
        const QString guid,
        const QString value);

    bool setTimeEntryBillable(
        const QString guid,
        const bool billable);

    bool setTimeEntryDate(
        const QString guid,
        const int64_t unix_timestamp);

    bool setTimeEntryStart(
        const QString guid,
        const QString value);

    bool setTimeEntryStop(
        const QString guid,
        const QString value);

    bool addProject(
        const QString time_entry_guid,
        const uint64_t workspace_id,
        const uint64_t client_id,
        const QString project_name,
        const bool is_private);

    bool setProxySettings(
        const bool useProxy,
        const QString proxyHost,
        const uint64_t proxyPort,
        const QString proxyUsername,
        const QString proxyPassword);

    bool setSettings(const bool useIdleDetection,
                     const bool menubarTimer,
                     const bool dockIcon,
                     const bool onTop,
                     const bool reminder);

    void toggleTimelineRecording(
        const bool recordTimeline);

    bool setUpdateChannel(const QString channel);

    bool sendFeedback(const QString topic,
                      const QString details,
                      const QString filename);

    bool discardTimeAt(const QString guid,
                       const uint64_t at);

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
        const bool is_online,
        const QString reason);

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
        const QString guid,
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

    bool setTime(
        const QString guid,
        const QDate date,
        const QString time,
        const bool start);
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_TOGGL_H_
