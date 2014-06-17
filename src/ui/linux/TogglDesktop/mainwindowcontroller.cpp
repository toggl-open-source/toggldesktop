#include "mainwindowcontroller.h"
#include "ui_mainwindowcontroller.h"

#include <QErrorMessage>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>
#include <QCloseEvent>

#include "kopsik_api.h"

MainWindowController::MainWindowController(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowController),
    ctx_(0),
    shutdown_(false)
{
    ui->setupUi(this);

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

    if (!kopsik_context_start_events(ctx_)) {
    }
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
    kopsik_context_clear(ctx_);
    delete ui;
}
