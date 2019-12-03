// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_TOGGL_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_TOGGL_H_

#include <QObject>
#include <QUrl>
#include <QVector>
#include <QQmlListProperty>
#include <QThread>

#include <stdint.h>

#include "./toggl_api.h"
#include "./autocompletelistmodel.h"
#include "./qmlobjectlist.h"

class AutocompleteView;
class GenericView;
class SettingsView;
class TimeEntryView;
class CountryView;
class TimeEntryViewStorage;

class TogglApi : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject*> countries READ countries NOTIFY countriesChanged)
    Q_PROPERTY(AutocompleteProxyModel* timeEntryAutocomplete READ timeEntryAutocomplete CONSTANT)
    Q_PROPERTY(AutocompleteProxyModel* minitimerAutocomplete READ minitimerAutocomplete CONSTANT)
    Q_PROPERTY(AutocompleteProxyModel* projectAutocomplete READ projectAutocomplete CONSTANT)
    Q_PROPERTY(TimeEntryViewStorage *timeEntries READ timeEntries CONSTANT)
    Q_PROPERTY(QStringList tags READ tags NOTIFY tagsChanged)

 public:
    TogglApi(
        QObject *parent = 0,
        QString logPathOverride = "",
        QString dbPathOverride = "");
    ~TogglApi();

    QThread *uiThread_;

    static TogglApi *instance;

    static QString Project;
    static QString Duration;
    static QString Description;

    static bool notifyBugsnag(
        const QString errorClass,
        const QString message,
        const QString context);

    bool shutdown;

    QList<QObject*> countries();
    AutocompleteProxyModel *timeEntryAutocomplete();
    AutocompleteProxyModel *minitimerAutocomplete();
    AutocompleteProxyModel *projectAutocomplete();
    TimeEntryViewStorage *timeEntries();
    void importTimeEntries(TogglTimeEntryView *first);
    QStringList tags();
signals:
    void countriesChanged();
    void timeEntryAutocompleteChanged();
    void minitimerAutocompleteChanged();
    void projectAutocompleteChanged();
    void tagsChanged();
private:
    QList<QObject*> countries_;
    AutocompleteListModel *timeEntryModel_;
    AutocompleteListModel *minitimerModel_;
    AutocompleteListModel *projectModel_;
    AutocompleteProxyModel *timeEntryAutocomplete_;
    AutocompleteProxyModel *minitimerAutocomplete_;
    AutocompleteProxyModel *projectAutocomplete_;
    TimeEntryViewStorage *timeEntries_;
    QStringList tags_;

 public slots:
    void setCountries(QVector<CountryView *> list);

    void displayTimeEntryAutocomplete(QVector<AutocompleteView *> list);
    void displayMinitimerAutocomplete(QVector<AutocompleteView *> list);
    void displayProjectAutocomplete(QVector<AutocompleteView *> list);

    void displayTags(QVector<GenericView *> list);

///////////////////////////////////////////////////////
    bool startEvents();

    void clear();

    void setEnvironment(const QString environment);

    void login(const QString email, const QString password);

    void signup(const QString email, const QString password,
                const uint64_t countryID);

    void googleLogin(const QString accessToken);

    void googleSignup(const QString &accessToken, uint64_t countryID);

    QString start(
        const QString description,
        const QString duration,
        const uint64_t task_id,
        const uint64_t project_id,
        const QString tags,
        const bool_t billable);

    bool stop();

    bool deleteTimeEntry(const QString guid);

    bool continueTimeEntry(const QString guid);

    bool continueLatestTimeEntry();

    void openInBrowser();

    void fullSync();

    void sync();

    bool clearCache();

    void getSupport();

    void logout();

    void editPreferences();

    void setOnline();

    void toggleEntriesGroup(const QString groupName);

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

    QString addProject(
        const QString time_entry_guid,
        const uint64_t workspace_id,
        const uint64_t client_id,
        const QString client_guid,
        const QString project_name,
        const bool is_private,
        const QString project_color);

    QString createClient(
        const uint64_t wid,
        const QString name);

    // returns false if error
    bool setSettingsAutodetectProxy(const bool value);

    // returns false on error
    bool setSettingsFocusOnShortcut(const bool value);

    bool setProxySettings(
        const bool useProxy,
        const QString proxyHost,
        const uint64_t proxyPort,
        const QString proxyUsername,
        const QString proxyPassword);

    bool setSettingsUseIdleDetection(const bool useIdleDetection);
    bool setSettingsReminder(const bool reminder);
    bool setSettingsIdleMinutes(const uint64_t idle_minutes);
    bool setSettingsReminderMinutes(const uint64_t reminder_minutes);
    bool setSettingsPomodoro(const bool pomodoro);
    bool setSettingsPomodoroMinutes(const uint64_t pomodoro_minutes);
    bool setSettingsPomodoroBreak(const bool pomodoro_break);
    bool setSettingsPomodoroBreakMinutes(const uint64_t pomodoro_break_minutes);
    bool setSettingsRemindDays(
        bool remind_mon,
        bool remind_tue,
        bool remind_wed,
        bool remind_thu,
        bool remind_fri,
        bool remind_sat,
        bool remind_sun);
    bool setSettingsRemindTimes (
        const QTime &remind_starts,
        const QTime &remind_ends);

    void stopEntryOnShutdown();
    bool setSettingsStopEntryOnShutdown(const bool stop_entry);

    void toggleTimelineRecording(
        const bool recordTimeline);

    bool setUpdateChannel(const QString channel);
    QString updateChannel();

    QString userEmail();

    // keyboard shortcut saving
    void setShowHideKey(const QString keys);
    void setContinueStopKey(const QString keys);
    QString getShowHideKey();
    QString getContinueStopKey();

    void getProjectColors();
    void getCountries();
    void loadMore();
    void tosAccept();
    void openLegal(const QString &link);

    bool sendFeedback(const QString topic,
                      const QString details,
                      const QString filename);

    bool discardTimeAt(const QString guid,
                       const uint64_t at,
                       const bool split_into_new_time_entry);

    bool discardTimeAndContinue(const QString guid,
                                const uint64_t at);

    bool runScriptFile(const QString filename);

    Q_INVOKABLE static const QString formatDurationInSecondsHHMMSS(
        const int64_t duration);

    QRect const getWindowsFrameSetting();
    void setWindowsFrameSetting(const QRect frame);

 signals:
    void displayApp(
        const bool open);

    void aboutToDisplayError();
    void displayError(
        const QString errmsg,
        const bool user_error);

    void aboutToDisplayOverlay();
    void displayOverlay(
        const int64_t type);

    void displayUpdate(
        const QString url);

    void displayOnlineState(
        int64_t state);

    void aboutToDisplayLogin();
    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void displayPomodoro(
        const QString title,
        const QString informative_text);

    void displayPomodoroBreak(
        const QString title,
        const QString informative_text);

    void displayReminder(
        const QString title,
        const QString informative_text);

    void aboutToDisplayTimeEntryList();
    void displayTimeEntryList(
        const bool open,
        const bool show_load_more_button);

    void aboutToDisplayTimeEntryEditor();
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
        const int64_t started,
        const QString description);

    void displayClientSelect(
        QVector<GenericView *> list);

    void displayWorkspaceSelect(
        QVector<GenericView *> list);

    void updateShowHideShortcut();

    void updateContinueStopShortcut();

    void setProjectColors(
        QVector<char *> list);

 private:
    void *ctx;

    bool setTime(
        const QString guid,
        const QDate date,
        const QString time,
        const bool start);
};

// callbacks used internally by the app instance
void on_display_app(const bool_t open);
void on_display_error(
    const char *errmsg,
    const bool_t user_error);
void on_overlay(const int64_t type);
void on_display_update(
    const char *url);
void on_display_online_state(
    const bool is_online,
    const char *reason);
void on_display_url(
    const char *url);
void on_display_login(
    const bool_t open,
    const uint64_t user_id);
void on_display_pomodoro(
    const char *title,
    const char *informative_text);
void on_display_pomodoro_break(
    const char *title,
    const char *informative_text);
void on_display_reminder(
    const char *title,
    const char *informative_text);
void on_display_time_entry_list(
    const bool_t open,
    TogglTimeEntryView *first);
void on_display_time_entry_autocomplete(
    TogglAutocompleteView *first);
void on_display_mini_timer_autocomplete(
    TogglAutocompleteView *first);
void on_display_project_autocomplete(
    TogglAutocompleteView *first);
void on_display_workspace_select(
    TogglGenericView *first);
void on_display_client_select(
    TogglGenericView *first);
void on_display_tags(
    TogglGenericView *first);
void on_display_time_entry_editor(
    const bool_t open,
    TogglTimeEntryView *te,
    const char *focused_field_name);
void on_display_settings(
    const bool_t open,
    TogglSettingsView *settings);
void on_display_timer_state(
    TogglTimeEntryView *te);
void on_display_idle_notification(
    const char *guid,
    const char *since,
    const char *duration,
    const int64_t started);
void on_project_colors(
    const char_t *list[],
    const uint64_t count);

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_TOGGL_H_
