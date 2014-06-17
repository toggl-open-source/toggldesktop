#include "mainwindowcontroller.h"
#include "ui_mainwindowcontroller.h"

#include <iostream>

#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>
#include <QCloseEvent>
#include <QMetaObject>
#include <QMessageBox>

#include "kopsik_api.h"

MainWindowController *MainWindowController::Instance = 0;

void on_display_app(const _Bool open) {
    const bool invoked = QMetaObject::invokeMethod(
                MainWindowController::Instance,
                "onDisplayApp",
                Qt::QueuedConnection,
                Q_ARG(_Bool, open));
    Q_ASSERT(invoked);
}

void MainWindowController::onDisplayApp(const _Bool open)
{

}

void on_display_error(
    const char *errmsg,
    const _Bool user_error)
{
    const bool invoked = QMetaObject::invokeMethod(
                MainWindowController::Instance,
                "onDisplayError",
                Qt::QueuedConnection,
                Q_ARG(QString, QString(errmsg)),
                Q_ARG(_Bool, user_error));
    Q_ASSERT(invoked);
}

void MainWindowController::onDisplayError(
    const QString errmsg,
    const _Bool user_error)
{

}

void on_display_update(
    const _Bool open,
    KopsikUpdateViewItem *update)
{

}

void MainWindowController::onDisplayUpdate(
    const _Bool open,
    KopsikUpdateViewItem *update)
{

}

void on_display_online_state(
    const _Bool is_online)
{
    const bool invoked = QMetaObject::invokeMethod(
                MainWindowController::Instance,
                "onDisplayOnlineState",
                Qt::QueuedConnection,
                Q_ARG(_Bool, is_online));
    Q_ASSERT(invoked);
}

void MainWindowController::onDisplayOnlineState(
    const _Bool is_online)
{
}

void on_display_url(
    const char *url)
{
    const bool invoked = QMetaObject::invokeMethod(
                MainWindowController::Instance,
                "onDisplayUrl",
                Qt::QueuedConnection,
                Q_ARG(QString, QString(url)));
    Q_ASSERT(invoked);
}

void MainWindowController::onDisplayUrl(
    const QString url)
{

}

void on_display_login(
    const _Bool open,
    const uint64_t user_id)
{
    const bool invoked = QMetaObject::invokeMethod(
                MainWindowController::Instance,
                "onDisplayLogin",
                Qt::QueuedConnection,
                Q_ARG(const bool, open),
                Q_ARG(const uint64_t, user_id));
    Q_ASSERT(invoked);
}

void MainWindowController::onDisplayLogin(
        const _Bool open,
        const uint64_t user_id)
{
}

void on_display_reminder(
    const char *title,
    const char *informative_text)
{
    const bool invoked = QMetaObject::invokeMethod(
                MainWindowController::Instance,
                "onDisplayReminder",
                Qt::QueuedConnection,
                Q_ARG(QString, QString(title)),
                Q_ARG(QString, QString(informative_text)));
    Q_ASSERT(invoked);
}

void MainWindowController::onDisplayReminder(
    const QString title,
    const QString informative_text)
{

}

void on_display_time_entry_list(
    const _Bool open,
    KopsikTimeEntryViewItem *first)
{

}

void MainWindowController::onDisplayTimeEntryList(
    const _Bool open,
    KopsikTimeEntryViewItem *first)
{

}

void on_display_time_entry_autocomplete(
    KopsikAutocompleteItem *first)
{

}

void MainWindowController::onDisplayTimeEntryAutocomplete(
    KopsikAutocompleteItem *first)
{

}

void on_display_project_autocomplete(
    KopsikAutocompleteItem *first)
{

}

void MainWindowController::onDisplayProjectAutocomplete(
        KopsikAutocompleteItem *first)
{

}

void on_display_workspace_select(
    KopsikViewItem *first)
{

}

void MainWindowController::onDisplayWorkspaceSelect(
    KopsikViewItem *first)
{

}

void on_display_client_select(
    KopsikViewItem *first)
{

}

void MainWindowController::onDisplayClientSelect(
    KopsikViewItem *first)
{

}

void on_display_tags(
    KopsikViewItem *first)
{

}

void MainWindowController::onDisplayTags(
    KopsikViewItem *first)
{

}

void on_display_time_entry_editor(
    const _Bool open,
    KopsikTimeEntryViewItem *te,
    const char *focused_field_name)
{

}

void MainWindowController::onDisplayTimeEntryEditor(
    const _Bool open,
    KopsikTimeEntryViewItem *te,
    const QString focused_field_name)
{

}

void on_display_settings(
    const _Bool open,
    KopsikSettingsViewItem *settings)
{

}

void MainWindowController::onDisplaySettings(
    const _Bool open,
    KopsikSettingsViewItem *settings)
{

}

void on_display_timer_state(
    KopsikTimeEntryViewItem *te)
{

}

void MainWindowController::onDisplayTimerState(
    KopsikTimeEntryViewItem *te)
{

}

void on_display_idle_notification(
    const char *since,
    const char *duration,
    const uint64_t started)
{
    const bool invoked = QMetaObject::invokeMethod(
                MainWindowController::Instance,
                "onDisplayIdleNotification",
                Qt::QueuedConnection,
                Q_ARG(QString, QString(since)),
                Q_ARG(QString, QString(duration)),
                Q_ARG(uint64_t, started));
    Q_ASSERT(invoked);
}

void MainWindowController::onDisplayIdleNotification(
    const QString since,
    const QString duration,
    const uint64_t started)
{

}

MainWindowController::MainWindowController(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowController),
    ctx_(0),
    shutdown_(false)
{
    ui->setupUi(this);

    Instance = this;

    readSettings();

    QString version = QCoreApplication::applicationVersion();
    ctx_ = kopsik_context_init("linux_native_app", version.toUtf8().constData());

    QString appDirPath =
            QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir appDir = QDir(appDirPath);
    if (!appDir.exists()) {
        appDir.mkpath(".");
    }

    QString logPath = appDir.filePath("kopsik.log");
    kopsik_set_log_path(logPath.toUtf8().constData());

    QString dbPath = appDir.filePath("kopsik.db");
    kopsik_set_db_path(ctx_, dbPath.toUtf8().constData());

    QString executablePath = QCoreApplication::applicationDirPath();
    QDir executableDir = QDir(executablePath);
    QString cacertPath = executableDir.filePath("cacert.pem");
    kopsik_set_cacert_path(ctx_, cacertPath.toUtf8().constData());

    kopsik_on_app(ctx_, on_display_app);
    kopsik_on_error(ctx_, on_display_error);
    kopsik_on_update(ctx_, on_display_update);
    kopsik_on_online_state(ctx_, on_display_online_state);
    kopsik_on_url(ctx_, on_display_url);
    kopsik_on_login(ctx_, on_display_login);
    kopsik_on_reminder(ctx_, on_display_reminder);
    kopsik_on_time_entry_list(ctx_, on_display_time_entry_list);
    kopsik_on_time_entry_autocomplete(ctx_, on_display_time_entry_autocomplete);
    kopsik_on_project_autocomplete(ctx_, on_display_project_autocomplete);
    kopsik_on_workspace_select(ctx_, on_display_workspace_select);
    kopsik_on_client_select(ctx_, on_display_client_select);
    kopsik_on_tags(ctx_, on_display_tags);
    kopsik_on_time_entry_editor(ctx_, on_display_time_entry_editor);
    kopsik_on_settings(ctx_, on_display_settings);
    kopsik_on_timer_state(ctx_, on_display_timer_state);
    kopsik_on_idle_notification(ctx_, on_display_idle_notification);

    _Bool started = kopsik_context_start_events(ctx_);
    Q_ASSERT(started);
}

void MainWindowController::readSettings()
{
    QSettings settings("Toggl", "TogglDesktop");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindowController::writeSettings()
{
    QSettings settings("Toggl", "TogglDesktop");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

void MainWindowController::closeEvent(QCloseEvent *event)
{
    writeSettings();
    if (!shutdown_) {
        event->ignore();
        hide();
        return;
    }
    QMainWindow::closeEvent(event);
}

MainWindowController::~MainWindowController()
{
    Instance = 0;
    kopsik_context_clear(ctx_);
    delete ui;
}
