#include "toggl_api.h"

#include <iostream>

#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDebug>

#include "kopsik_api.h"

#include "updateview.h"
#include "timeentryview.h"
#include "genericview.h"
#include "autocompleteview.h"
#include "settingsview.h"

TogglApi *TogglApi::instance = 0;

QString TogglApi::Project = QString("project");
QString TogglApi::Duration = QString("duration");
QString TogglApi::Description = QString("description");

void on_display_app(const _Bool open) {
    TogglApi::instance->displayApp(open);
}

void on_display_error(
    const char *errmsg,
    const _Bool user_error)
{
    TogglApi::instance->displayError(QString(errmsg), user_error);
}

void on_display_update(
    const _Bool open,
    KopsikUpdateViewItem *update)
{
    TogglApi::instance->displayUpdate(open,
                                      UpdateView::importOne(update));
}

void on_display_online_state(
    const bool is_online)
{
    TogglApi::instance->displayOnlineState(is_online);
}

void on_display_url(
    const char *url)
{
    QDesktopServices::openUrl(QUrl(url));
}

void on_display_login(
    const _Bool open,
    const uint64_t user_id)
{
    TogglApi::instance->displayLogin(open, user_id);
}

void on_display_reminder(
    const char *title,
    const char *informative_text)
{
    TogglApi::instance->displayReminder(
                QString(title),
                QString(informative_text));
}

void on_display_time_entry_list(
    const _Bool open,
    KopsikTimeEntryViewItem *first)
{
    TogglApi::instance->displayTimeEntryList(
                open,
                TimeEntryView::importAll(first));
}

void on_display_time_entry_autocomplete(
    KopsikAutocompleteItem *first)
{
    TogglApi::instance->displayTimeEntryAutocomplete(
                AutocompleteView::importAll(first));
}

void on_display_project_autocomplete(
    KopsikAutocompleteItem *first)
{
    TogglApi::instance->displayProjectAutocomplete(
                AutocompleteView::importAll(first));
}

void on_display_workspace_select(
    KopsikViewItem *first)
{
    TogglApi::instance->displayWorkspaceSelect(
                GenericView::importAll(first));
}

void on_display_client_select(
    KopsikViewItem *first)
{
    TogglApi::instance->displayClientSelect(
                GenericView::importAll(first));
}

void on_display_tags(
    KopsikViewItem *first)
{
    TogglApi::instance->displayTags(
                GenericView::importAll(first));
}

void on_display_time_entry_editor(
    const _Bool open,
    KopsikTimeEntryViewItem *te,
    const char *focused_field_name)
{
    TogglApi::instance->displayTimeEntryEditor(
                open,
                TimeEntryView::importOne(te),
                QString(focused_field_name));
}

void on_display_settings(
    const _Bool open,
    KopsikSettingsViewItem *settings)
{
    TogglApi::instance->displaySettings(
                open,
                SettingsView::importOne(settings));
}

void on_display_timer_state(
    KopsikTimeEntryViewItem *te)
{
    if (te) {
        TogglApi::instance->displayRunningTimerState(
                    TimeEntryView::importOne(te));
        return;
    }

    TogglApi::instance->displayStoppedTimerState();
}

void on_display_idle_notification(
    const char *since,
    const char *duration,
    const uint64_t started)
{
    TogglApi::instance->displayIdleNotification(
                QString(since),
                QString(duration),
                started);
}

TogglApi::TogglApi(QObject *parent) :
    QObject(parent),
    ctx(0)
{
    ctx = kopsik_context_init("linux_native_app", "7.0.0"); // FIXME: version number

    QString appDirPath =
            QStandardPaths::writableLocation(
                QStandardPaths::DataLocation);
    QDir appDir = QDir(appDirPath);
    if (!appDir.exists()) {
        appDir.mkpath(".");
    }

    QString logPath = appDir.filePath("toggldesktop.log");
    kopsik_set_log_path(logPath.toUtf8().constData());
    qDebug() << "Log path " << logPath;

    QString dbPath = appDir.filePath("toggldesktop.db");
    kopsik_set_db_path(ctx, dbPath.toUtf8().constData());
    qDebug() << "DB path " << dbPath;

    QString executablePath = QCoreApplication::applicationDirPath();
    QDir executableDir = QDir(executablePath);
    QString cacertPath = executableDir.filePath("cacert.pem");
    kopsik_set_cacert_path(ctx, cacertPath.toUtf8().constData());

    kopsik_on_app(ctx, on_display_app);
    kopsik_on_error(ctx, on_display_error);
    kopsik_on_update(ctx, on_display_update);
    kopsik_on_online_state(ctx, on_display_online_state);
    kopsik_on_url(ctx, on_display_url);
    kopsik_on_login(ctx, on_display_login);
    kopsik_on_reminder(ctx, on_display_reminder);
    kopsik_on_time_entry_list(ctx, on_display_time_entry_list);
    kopsik_on_time_entry_autocomplete(ctx, on_display_time_entry_autocomplete);
    kopsik_on_project_autocomplete(ctx, on_display_project_autocomplete);
    kopsik_on_workspace_select(ctx, on_display_workspace_select);
    kopsik_on_client_select(ctx, on_display_client_select);
    kopsik_on_tags(ctx, on_display_tags);
    kopsik_on_time_entry_editor(ctx, on_display_time_entry_editor);
    kopsik_on_settings(ctx, on_display_settings);
    kopsik_on_timer_state(ctx, on_display_timer_state);
    kopsik_on_idle_notification(ctx, on_display_idle_notification);

    instance = this;
}

bool TogglApi::startEvents()
{
    return kopsik_context_start_events(ctx);
}

void TogglApi::login(const QString email, const QString password)
{
    kopsik_login(ctx, email.toStdString().c_str(), password.toStdString().c_str());
}

void TogglApi::googleLogin(const QString accessToken)
{
    kopsik_google_login(ctx, accessToken.toStdString().c_str());
}

bool TogglApi::setProxySettings(
    const bool useProxy,
    const QString proxyHost,
    const uint64_t proxyPort,
    const QString proxyUsername,
    const QString proxyPassword)
{
    return kopsik_set_proxy_settings(ctx,
                                     useProxy,
                                     proxyHost.toStdString().c_str(),
                                     proxyPort,
                                     proxyUsername.toStdString().c_str(),
                                     proxyPassword.toStdString().c_str());
}

bool TogglApi::stopRunningTimeEntryAt(const uint64_t at)
{
    return kopsik_stop_running_time_entry_at(ctx, at);
}

void TogglApi::setIdleSeconds(u_int64_t idleSeconds)
{
    kopsik_set_idle_seconds(ctx, idleSeconds);
}

bool TogglApi::setSettings(const bool useIdleDetection,
                           const bool menubarTimer,
                           const bool dockIcon,
                           const bool onTop,
                           const bool reminder,
                           const bool ignoreCert)
{
    return kopsik_set_settings(ctx,
                               useIdleDetection,
                               menubarTimer,
                               dockIcon,
                               onTop,
                               reminder,
                               ignoreCert);
}

void TogglApi::toggleTimelineRecording()
{
    kopsik_timeline_toggle_recording(ctx);
}

bool TogglApi::setUpdateChannel(const QString channel)
{
    return kopsik_set_update_channel(ctx, channel.toStdString().c_str());
}

bool TogglApi::start(
    const QString description,
    const QString duration,
    const uint64_t task_id,
    const uint64_t project_id)
{
    return kopsik_start(ctx,
                       description.toStdString().c_str(),
                       duration.toStdString().c_str(),
                       task_id,
                       project_id);
}

bool TogglApi::stop()
{
    return kopsik_stop(ctx);
}

const QString TogglApi::formatDurationInSecondsHHMMSS(const int64_t duration)
{
    int bufSize = 20;
    char buf[bufSize];
    kopsik_format_duration_in_seconds_hhmmss(duration,
                                             buf,
                                             bufSize);
    return QString(buf);
}

bool TogglApi::continueTimeEntry(const QString guid)
{
    return kopsik_continue(ctx, guid.toStdString().c_str());
}

bool TogglApi::continueLatestTimeEntry()
{
    return kopsik_continue_latest(ctx);
}

void TogglApi::sync()
{
    kopsik_sync(ctx);
}

void TogglApi::openInBrowser()
{
    kopsik_open_in_browser(ctx);
}

void TogglApi::about()
{
    kopsik_about(ctx);
}

bool TogglApi::clearCache()
{
    return kopsik_clear_cache(ctx);
}

void TogglApi::getSupport()
{
    kopsik_get_support(ctx);
}

void TogglApi::logout()
{
    kopsik_logout(ctx);
}

void TogglApi::editPreferences()
{
    kopsik_edit_preferences(ctx);
}

void TogglApi::editTimeEntry(const QString guid,
                    const QString focusedFieldName)
{
    kopsik_edit(ctx,
                guid.toStdString().c_str(),
                false,
                focusedFieldName.toStdString().c_str());
}

bool TogglApi::setTimeEntryProject(
        const QString guid,
        const uint64_t task_id,
        const uint64_t project_id,
        const QString project_guid)
{
    return kopsik_set_time_entry_project(ctx,
                                         guid.toStdString().c_str(),
                                         task_id,
                                         project_id,
                                         project_guid.toStdString().c_str());
}

bool TogglApi::setTimeEntryDescription(
        const QString guid,
        const QString value)
{
    return kopsik_set_time_entry_description(ctx,
                                             guid.toStdString().c_str(),
                                             value.toStdString().c_str());
}

void TogglApi::editRunningTimeEntry(
                    const QString focusedFieldName)
{
    kopsik_edit(ctx,
                "",
                true,
                focusedFieldName.toStdString().c_str());
}

bool TogglApi::setTimeEntryBillable(
    const QString guid,
    const bool billable)
{
    return kopsik_set_time_entry_billable(ctx,
                                          guid.toStdString().c_str(),
                                          billable);
}

bool TogglApi::addProject(
    const QString time_entry_guid,
    const uint64_t workspace_id,
    const uint64_t client_id,
    const QString project_name,
    const bool is_private)
{
    return kopsik_add_project(ctx,
                              time_entry_guid.toStdString().c_str(),
                              workspace_id,
                              client_id,
                              project_name.toStdString().c_str(),
                              is_private);
}

void TogglApi::viewTimeEntryList()
{
    kopsik_view_time_entry_list(ctx);
}

bool TogglApi::deleteTimeEntry(const QString guid)
{
    return kopsik_delete_time_entry(ctx, guid.toStdString().c_str());
}

bool TogglApi::setTimeEntryDuration(
    const QString guid,
    const QString value)
{
    return kopsik_set_time_entry_duration(ctx,
                                          guid.toStdString().c_str(),
                                          value.toStdString().c_str());
}

bool TogglApi::sendFeedback(const QString topic,
                            const QString details,
                            const QString filename)
{
    return kopsik_feedback_send(ctx,
                                topic.toStdString().c_str(),
                                details.toStdString().c_str(),
                                filename.toStdString().c_str());
}
