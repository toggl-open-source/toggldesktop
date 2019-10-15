// Copyright 2014 Toggl Desktop developers.

#include "./toggl.h"

#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDebug>
#include <QApplication>
#include <QStringList>
#include <QDate>

#include <iostream>   // NOLINT

#include "./../../../toggl_api.h"
#include "./../../../platforminfo.h"

#include "./timeentryview.h"
#include "./genericview.h"
#include "./countryview.h"
#include "./autocompleteview.h"
#include "./settingsview.h"
#include "./bugsnag.h"

TogglApi *TogglApi::instance = nullptr;

QString TogglApi::Project = QString("project");
QString TogglApi::Duration = QString("duration");
QString TogglApi::Description = QString("description");

void on_display_app(const bool_t open) {
    TogglApi::instance->displayApp(open);
}

void on_display_update(const char_t *url) {
    TogglApi::instance->displayUpdate(toQString(url));
}

void on_display_error(
    const char_t *errmsg,
    const bool_t user_error) {
    TogglApi::instance->aboutToDisplayError();
    TogglApi::instance->displayError(toQString(errmsg), user_error);
}

void on_overlay(const int64_t type) {
    TogglApi::instance->aboutToDisplayOverlay();
    TogglApi::instance->displayOverlay(type);
}

void on_display_online_state(
    int64_t state) {
    TogglApi::instance->displayOnlineState(state);
}

void on_display_url(
    const char_t *url) {
    QDesktopServices::openUrl(QUrl(toQString(url)));
}

void on_display_login(
    const bool_t open,
    const uint64_t user_id) {
    if (open) {
        TogglApi::instance->aboutToDisplayLogin();
    }
    TogglApi::instance->getCountries();
    TogglApi::instance->displayLogin(open, user_id);
    Bugsnag::user.id = QString("%1").arg(user_id);
}

void on_display_pomodoro(
    const char_t *title,
    const char_t *informative_text) {
    TogglApi::instance->displayPomodoro(
        toQString(title),
        toQString(informative_text));
}

void on_display_pomodoro_break(
    const char_t *title,
    const char_t *informative_text) {
    TogglApi::instance->displayPomodoroBreak(
        toQString(title),
        toQString(informative_text));
}

void on_display_reminder(
    const char_t *title,
    const char_t *informative_text) {
    TogglApi::instance->displayReminder(
        toQString(title),
        toQString(informative_text));
}

void on_display_time_entry_list(
    const bool_t open,
    TogglTimeEntryView *first,
    const bool_t show_load_more_button) {
    if (open) {
        TogglApi::instance->aboutToDisplayTimeEntryList();
    }
    TogglApi::instance->importTimeEntries(TimeEntryView::importAll(first));
    TogglApi::instance->displayTimeEntryList(
        open,
        show_load_more_button);

}

void on_display_time_entry_autocomplete(
    TogglAutocompleteView *first) {
    TogglApi::instance->displayTimeEntryAutocomplete(
        AutocompleteView::importAll(first));
}

void on_display_mini_timer_autocomplete(
    TogglAutocompleteView *first) {
    TogglApi::instance->displayMinitimerAutocomplete(
        AutocompleteView::importAll(first));
}

void on_display_project_autocomplete(
    TogglAutocompleteView *first) {
    TogglApi::instance->displayProjectAutocomplete(
        AutocompleteView::importAll(first));
}

void on_display_workspace_select(
    TogglGenericView *first) {
    TogglApi::instance->displayWorkspaceSelect(
        GenericView::importAll(first));
}

void on_display_client_select(
    TogglGenericView *first) {
    TogglApi::instance->displayClientSelect(
        GenericView::importAll(first));
}

void on_display_tags(
    TogglGenericView *first) {
    TogglApi::instance->displayTags(
        GenericView::importAll(first));
}

void on_display_time_entry_editor(
    const bool_t open,
    TogglTimeEntryView *te,
    const char_t *focused_field_name) {
    if (open) {
        TogglApi::instance->aboutToDisplayTimeEntryEditor();
    }
    TogglApi::instance->displayTimeEntryEditor(
        open,
        TimeEntryView::importOne(te),
        toQString(focused_field_name));
}

void on_display_settings(
    const bool_t open,
    TogglSettingsView *settings) {
    TogglApi::instance->displaySettings(
        open,
        SettingsView::importOne(settings));
}

void on_display_timer_state(
    TogglTimeEntryView *te) {
    if (te) {
        TogglApi::instance->displayRunningTimerState(
            TimeEntryView::importOne(te));
        return;
    }

    TogglApi::instance->displayStoppedTimerState();
}

void on_display_idle_notification(
    const char_t *guid,
    const char_t *since,
    const char_t *duration,
    const int64_t started,
    const char_t *description,
    const char_t *project,
    const char_t *task,
    const char_t *projectColor) {
    TogglApi::instance->displayIdleNotification(
        toQString(guid),
        toQString(since),
        toQString(duration),
        started,
        toQString(description));
}


void on_project_colors(
    char_t *list[],
    const uint64_t count)
{
    QVector<QString> result;
    for (uint i = 0; i < count; i++)
    {
        char_t *c = list[i];
        result.push_back(toQString(c));
    }
    TogglApi::instance->setProjectColors(result);
}

void on_countries(
    TogglCountryView *first) {
    TogglApi::instance->setCountries(
        CountryView::importAll(first));
}

void TogglApi::setCountries(QVector<CountryView *> list) {
    for (auto i : countries_)
        i->deleteLater();
    countries_.clear();
    for (auto i : list) {
        countries_.append(i);
    }
    emit countriesChanged();
}

TogglApi::TogglApi(
    QObject *parent,
    QString logPathOverride,
    QString dbPathOverride)
    : QObject(parent)
, shutdown(false)
, ctx(nullptr) {
    QString version = QApplication::applicationVersion();

    QString appDirPath =
        QStandardPaths::writableLocation(
            QStandardPaths::DataLocation);
    QDir appDir = QDir(appDirPath);
    if (!appDir.exists()) {
        appDir.mkpath(".");
    }

    QString logPath("");
    if (logPathOverride.isEmpty()) {
        if (toggl_get_server_type() == TogglServerStaging)
            logPath = appDir.filePath("toggldesktop-staging.log");
        else
            logPath = appDir.filePath("toggldesktop.log");
    } else {
        logPath = logPathOverride;
    }
    toggl_set_log_path(toCStr(logPath));
    qDebug() << "Log path " << logPath;

    toggl_set_log_level(toCStr("debug"));
    ctx = toggl_context_init(toCStr("linux_native_app"),
                             toCStr(version));

    QString dbPath("");
    if (dbPathOverride.isEmpty()) {
        if (toggl_get_server_type() == TogglServerStaging)
            dbPath = appDir.filePath("toggldesktop-staging.db");
        else
            dbPath = appDir.filePath("toggldesktop.db");
    } else {
        dbPath = dbPathOverride;
    }
    toggl_set_db_path(ctx, toCStr(dbPath));
    qDebug() << "DB path " << dbPath;

    QString executablePath = QCoreApplication::applicationDirPath();
    QDir executableDir = QDir(executablePath);
    QString cacertPath = executableDir.filePath("cacert.pem");
    if (!QFile::exists(cacertPath)) {
        cacertPath = QString("%1/../share/toggldesktop/cacert.pem").arg(executableDir.path());
    }
#ifdef TOGGL_DATA_DIR
    if (!QFile::exists(cacertPath)) {
        cacertPath = QString("%1/cacert.pem").arg(TOGGL_DATA_DIR);
    }
#endif // TOGGL_DATA_DIR
    toggl_set_cacert_path(ctx, toCStr(cacertPath));

    toggl_on_show_app(ctx, on_display_app);
    toggl_on_update(ctx, on_display_update);
    toggl_on_error(ctx, on_display_error);
    toggl_on_overlay(ctx, on_overlay);
    toggl_on_online_state(ctx, on_display_online_state);
    toggl_on_url(ctx, on_display_url);
    toggl_on_login(ctx, on_display_login);
    toggl_on_pomodoro(ctx, on_display_pomodoro);
    toggl_on_pomodoro_break(ctx, on_display_pomodoro_break);
    toggl_on_reminder(ctx, on_display_reminder);
    toggl_on_time_entry_list(ctx, on_display_time_entry_list);
    toggl_on_time_entry_autocomplete(ctx, on_display_time_entry_autocomplete);
    toggl_on_mini_timer_autocomplete(ctx, on_display_mini_timer_autocomplete);
    toggl_on_project_autocomplete(ctx, on_display_project_autocomplete);
    toggl_on_workspace_select(ctx, on_display_workspace_select);
    toggl_on_client_select(ctx, on_display_client_select);
    toggl_on_tags(ctx, on_display_tags);
    toggl_on_time_entry_editor(ctx, on_display_time_entry_editor);
    toggl_on_settings(ctx, on_display_settings);
    toggl_on_timer_state(ctx, on_display_timer_state);
    toggl_on_idle_notification(ctx, on_display_idle_notification);
    toggl_on_project_colors(ctx, on_project_colors);
    toggl_on_countries(ctx, on_countries);

    char_t *env = toggl_environment(ctx);
    if (env) {
        Bugsnag::releaseStage = toQString(env);
        free(env);
    }

    Bugsnag::device.osName = "linux";
    auto platform = RetrieveOsDetailsMap();
    if (platform.count("window_manager"))
        Bugsnag::device.wm = QString::fromStdString(platform["window_manager"]);
    if (platform.count("desktop_environment"))
        Bugsnag::device.de = QString::fromStdString(platform["desktop_environment"]);
    if (platform.count("distribution") && platform.count("distribution_version"))
        Bugsnag::device.osVersion = QString::fromStdString(platform["distribution"] + " " + platform["distribution_version"]);
    if (platform.count("session_type"))
        Bugsnag::device.session = QString::fromStdString(platform["session_type"]);
    if(platform.count("build_type"))
        Bugsnag::device.build = QString::fromStdString(platform["build_type"]);

    instance = this;
}

TogglApi::~TogglApi() {
    instance = nullptr;
}

bool TogglApi::notifyBugsnag(
    const QString errorClass,
    const QString message,
    const QString context) {
    QHash<QString, QHash<QString, QString> > metadata;
    if (instance) {
        metadata["release"]["channel"] = instance->updateChannel();
    }
    return Bugsnag::notify(errorClass, message, context, &metadata);
}

QList<QObject*> TogglApi::countries() {
    return countries_;
}

QQmlListProperty<TimeEntryView> TogglApi::timeEntries() {
    return QQmlListProperty<TimeEntryView>(this, timeEntries_);
}

void TogglApi::importTimeEntries(QVector<TimeEntryView *> list) {
    for (auto i : timeEntries_)
        i->deleteLater();
    timeEntries_.clear();
    for (auto i : list) {
        timeEntries_.append(i);
    }
    emit timeEntriesChanged();
}

bool TogglApi::startEvents() {
    return toggl_ui_start(ctx);
}

void TogglApi::clear() {
    toggl_context_clear(ctx);
    ctx = nullptr;

    instance = nullptr;
}

void TogglApi::login(const QString email, const QString password) {
    toggl_login_async(ctx,
                      toCStr(email),
                      toCStr(password));
}

void TogglApi::signup(const QString email, const QString password,
                      const uint64_t countryID) {
    toggl_signup_async(ctx,
                       toCStr(email),
                       toCStr(password),
                       countryID);
}

void TogglApi::setEnvironment(const QString environment) {
    toggl_set_environment(ctx, toCStr(environment));
    Bugsnag::releaseStage = environment;
}

bool TogglApi::setTimeEntryDate(
    const QString guid,
    const int64_t unix_timestamp) {
    return toggl_set_time_entry_date(ctx,
                                     toCStr(guid),
                                     unix_timestamp);
}

bool TogglApi::setTimeEntryStart(
    const QString guid,
    const QString value) {
    return toggl_set_time_entry_start(ctx,
                                      toCStr(guid),
                                      toCStr(value));
}

bool TogglApi::setTimeEntryStop(
    const QString guid,
    const QString value) {
    return toggl_set_time_entry_end(ctx,
                                    toCStr(guid),
                                    toCStr(value));
}

void TogglApi::googleLogin(const QString accessToken) {
    toggl_google_login_async(ctx, toCStr(accessToken));
}

void TogglApi::googleSignup(const QString &accessToken, uint64_t countryID) {
    toggl_google_signup_async(ctx, toCStr(accessToken), countryID);
}

bool TogglApi::setProxySettings(
    const bool useProxy,
    const QString proxyHost,
    const uint64_t proxyPort,
    const QString proxyUsername,
    const QString proxyPassword) {
    return toggl_set_proxy_settings(ctx,
                                    useProxy,
                                    toCStr(proxyHost),
                                    proxyPort,
                                    toCStr(proxyUsername),
                                    toCStr(proxyPassword));
}

bool TogglApi::discardTimeAt(const QString guid,
                             const uint64_t at,
                             const bool split_into_new_time_entry) {
    return toggl_discard_time_at(ctx,
                                 toCStr(guid),
                                 at,
                                 split_into_new_time_entry);
}

bool TogglApi::discardTimeAndContinue(const QString guid,
                                      const uint64_t at) {
    return toggl_discard_time_and_continue(ctx, toCStr(guid), at);
}

void TogglApi::setIdleSeconds(uint64_t idleSeconds) {
    toggl_set_idle_seconds(ctx, idleSeconds);
}

bool TogglApi::setSettingsUseIdleDetection(const bool useIdleDetection) {
    return toggl_set_settings_use_idle_detection(ctx,
            useIdleDetection);
}

bool TogglApi::setSettingsAutodetectProxy(const bool value) {
    return toggl_set_settings_autodetect_proxy(ctx, value);
}

bool TogglApi::setSettingsFocusOnShortcut(const bool value) {
    return toggl_set_settings_focus_on_shortcut(ctx, value);
}

bool TogglApi::setSettingsReminder(const bool reminder) {
    return toggl_set_settings_reminder(ctx,
                                       reminder);
}

bool TogglApi::setSettingsPomodoro(const bool pomodoro) {
    return toggl_set_settings_pomodoro(ctx,
                                       pomodoro);
}

bool TogglApi::setSettingsPomodoroBreak(const bool pomodoro_break) {
    return toggl_set_settings_pomodoro_break(ctx,
            pomodoro_break);
}

bool TogglApi::setSettingsIdleMinutes(const uint64_t idleMinutes) {
    return toggl_set_settings_idle_minutes(ctx,
                                           idleMinutes);
}

bool TogglApi::setSettingsReminderMinutes(const uint64_t reminderMinutes) {
    return toggl_set_settings_reminder_minutes(ctx,
            reminderMinutes);
}

bool TogglApi::setSettingsPomodoroMinutes(const uint64_t pomodoroMinutes) {
    return toggl_set_settings_pomodoro_minutes(ctx,
            pomodoroMinutes);
}

bool TogglApi::setSettingsPomodoroBreakMinutes(
    const uint64_t pomodoro_break_minutes) {
    return toggl_set_settings_pomodoro_break_minutes(ctx,
            pomodoro_break_minutes);
}

bool TogglApi::setSettingsRemindDays(
    bool remind_mon,
    bool remind_tue,
    bool remind_wed,
    bool remind_thu,
    bool remind_fri,
    bool remind_sat,
    bool remind_sun)
{
    return toggl_set_settings_remind_days(ctx,
                                          remind_mon,
                                          remind_tue,
                                          remind_wed,
                                          remind_thu,
                                          remind_fri,
                                          remind_sat,
                                          remind_sun);
}

bool TogglApi::setSettingsRemindTimes(const QTime &remind_starts, const QTime &remind_ends) {
    return toggl_set_settings_remind_times(ctx, toCStr(remind_starts.toString("HH:mm")), toCStr(remind_ends.toString("HH:mm")));
}

void TogglApi::toggleTimelineRecording(const bool recordTimeline) {
    toggl_timeline_toggle_recording(ctx, recordTimeline);
}

bool TogglApi::setUpdateChannel(const QString channel) {
    return toggl_set_update_channel(ctx, toCStr(channel));
}

bool TogglApi::setSettingsStopEntryOnShutdown(const bool stop_entry) {
    return toggl_set_settings_stop_entry_on_shutdown_sleep(ctx, stop_entry);
}

void TogglApi::stopEntryOnShutdown() {
    toggl_os_shutdown(ctx);
}

QString TogglApi::updateChannel() {
    char_t *channel = toggl_get_update_channel(ctx);
    QString res;
    if (channel) {
        res = toQString(channel);
        free(channel);
    }
    return res;
}

QString TogglApi::userEmail() {
    char_t *email = toggl_get_user_email(ctx);
    QString res;
    if (email) {
        res = toQString(email);
        free(email);
    }
    return res;
}

QString TogglApi::start(
    const QString &description,
    const QString &duration,
    const uint64_t task_id,
    const uint64_t project_id,
    const QString &tags,
    const bool_t billable) {

    char_t *guid = toggl_start(ctx,
                               toCStr(description),
                               toCStr(duration),
                               task_id,
                               project_id,
                               nullptr /* project guid */,
                               toCStr(tags) /* tags */,
                               false,
                               0,
                               0);
    QString res("");
    if (guid) {
        res = toQString(guid);
        free(guid);
        if (billable) {
            toggl_set_time_entry_billable(ctx,
                                          toCStr(res),
                                          billable);
        }
    }
    return res;
}

bool TogglApi::stop() {
    return toggl_stop(ctx, false);
}

const QString TogglApi::formatDurationInSecondsHHMMSS(
    const int64_t duration) {
    char_t *buf = toggl_format_tracking_time_duration(duration);
    QString res = toQString(buf);
    free(buf);
    return res;
}

bool TogglApi::continueTimeEntry(const QString guid) {
    return toggl_continue(ctx, toCStr(guid));
}

bool TogglApi::continueLatestTimeEntry() {
    return toggl_continue_latest(ctx, false);
}

void TogglApi::fullSync() {
    toggl_fullsync(ctx);
}

void TogglApi::sync() {
    toggl_sync(ctx);
}

void TogglApi::openInBrowser() {
    toggl_open_in_browser(ctx);
}

bool TogglApi::clearCache() {
    return toggl_clear_cache(ctx);
}

void TogglApi::getSupport() {
    toggl_get_support(ctx, 2);
}

void TogglApi::logout() {
    toggl_logout(ctx);
}

void TogglApi::editPreferences() {
    toggl_edit_preferences(ctx);
}

void TogglApi::setOnline() {
    toggl_set_online(ctx);
}

void TogglApi::toggleEntriesGroup(const QString groupName) {
    toggl_toggle_entries_group(ctx, toCStr(groupName));
}

void TogglApi::editTimeEntry(const QString guid,
                             const QString focusedFieldName) {
    toggl_edit(ctx,
               toCStr(guid),
               false,
               toCStr(focusedFieldName));
}

bool TogglApi::setTimeEntryProject(
    const QString guid,
    const uint64_t task_id,
    const uint64_t project_id,
    const QString project_guid) {
    return toggl_set_time_entry_project(ctx,
                                        toCStr(guid),
                                        task_id,
                                        project_id,
                                        toCStr(project_guid));
}

bool TogglApi::setTimeEntryDescription(
    const QString guid,
    const QString value) {
    return toggl_set_time_entry_description(ctx,
                                            toCStr(guid),
                                            toCStr(value));
}

bool TogglApi::setTimeEntryTags(
    const QString guid,
    const QString tags) {
    return toggl_set_time_entry_tags(ctx,
                                     toCStr(guid),
                                     toCStr(tags));
}

void TogglApi::editRunningTimeEntry(
    const QString focusedFieldName) {
    toggl_edit(ctx,
               toCStr(""),
               true,
               toCStr(focusedFieldName));
}

bool TogglApi::setTimeEntryBillable(
    const QString guid,
    const bool billable) {
    return toggl_set_time_entry_billable(ctx,
                                         toCStr(guid),
                                         billable);
}

QString TogglApi::addProject(
    const QString time_entry_guid,
    const uint64_t workspace_id,
    const uint64_t client_id,
    const QString client_guid,
    const QString project_name,
    const bool is_private,
    const QString project_color) {

    char_t *guid = toggl_add_project(ctx,
                                     toCStr(time_entry_guid),
                                     workspace_id,
                                     client_id,
                                     toCStr(client_guid),
                                     toCStr(project_name),
                                     is_private,
                                     toCStr(project_color));
    QString res("");
    if (guid) {
        res = toQString(guid);
        free(guid);
    }
    return res;
}

QString TogglApi::createClient(
    const uint64_t wid,
    const QString name) {
    char_t *guid = toggl_create_client(ctx,
                                       wid,
                                       toCStr(name));
    QString res("");
    if (guid) {
        res = toQString(guid);
        free(guid);
    }
    return res;
}

void TogglApi::viewTimeEntryList() {
    toggl_view_time_entry_list(ctx);
}

bool TogglApi::deleteTimeEntry(const QString guid) {
    return toggl_delete_time_entry(ctx, toCStr(guid));
}

bool TogglApi::setTimeEntryDuration(
    const QString guid,
    const QString value) {
    return toggl_set_time_entry_duration(ctx,
                                         toCStr(guid),
                                         toCStr(value));
}

bool TogglApi::sendFeedback(const QString topic,
                            const QString details,
                            const QString filename) {
    return toggl_feedback_send(ctx,
                               toCStr(topic),
                               toCStr(details),
                               toCStr(filename));
}

void TogglApi::setShowHideKey(const QString keys) {
    toggl_set_key_show(ctx, toCStr(keys));
    TogglApi::instance->updateShowHideShortcut();
}

void TogglApi::setContinueStopKey(const QString keys) {
    toggl_set_key_start(ctx, toCStr(keys));
    TogglApi::instance->updateContinueStopShortcut();
}

QString TogglApi::getShowHideKey() {
    char_t *buf = toggl_get_key_show(ctx);
    QString res = toQString(buf);
    free(buf);
    return res;
}

QString TogglApi::getContinueStopKey() {
    char_t *buf = toggl_get_key_start(ctx);
    QString res = toQString(buf);
    free(buf);
    return res;
}

void TogglApi::getProjectColors() {
    toggl_get_project_colors(ctx);
}

void TogglApi::getCountries() {
    toggl_get_countries(ctx);
}

void TogglApi::loadMore() {
    toggl_load_more(ctx);
}

void TogglApi::tosAccept() {
    toggl_accept_tos(ctx);
}

void TogglApi::openLegal(const QString &link) {
    if (link == "terms") {
        toggl_tos(ctx);
    } else if (link == "privacy") {
        toggl_privacy_policy(ctx);
    }
}

QRect const TogglApi::getWindowsFrameSetting() {
    int64_t x;
    int64_t y;
    int64_t w;
    int64_t h;
    toggl_window_settings(ctx, &x, &y, &h, &w);
    return QRect(static_cast<int>(x),
                 static_cast<int>(y),
                 static_cast<int>(w),
                 static_cast<int>(h));
}

void TogglApi::setWindowsFrameSetting(const QRect frame) {
    toggl_set_window_settings(ctx,
                              frame.x(),
                              frame.y(),
                              frame.height(),
                              frame.width());
}
