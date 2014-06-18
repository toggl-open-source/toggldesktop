#include "mainwindowcontroller.h"
#include "ui_mainwindowcontroller.h"

#include <iostream>

#include <QSettings>
#include <QCloseEvent>
#include <QMessageBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <qdesktopservices.h>

#include "toggl_api.h"
#include "errorviewcontroller.h"
#include "loginwidget.h"

MainWindowController::MainWindowController(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowController),
    ctx_(0),
    shutdown_(false),
    togglApi(new TogglApi()),
    stackedWidget(0),
    loginWidget(0)
{
    ui->setupUi(this);

    ui->mainToolBar->setVisible(false);

    loginWidget = new LoginWidget();

    QVBoxLayout *verticalLayout = new QVBoxLayout();
    verticalLayout->addWidget(new ErrorViewController());
    verticalLayout->addWidget(loginWidget);
    centralWidget()->setLayout(verticalLayout);

    readSettings();

    connect(TogglApi::instance, SIGNAL(displayApp(bool)), this, SLOT(displayApp(bool)));
    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)), this, SLOT(displayLogin(bool,uint64_t)));
    connect(TogglApi::instance, SIGNAL(displayUrl(QUrl)), this, SLOT(displayUrl(QUrl)));
}

MainWindowController::~MainWindowController()
{
    delete togglApi;
    togglApi = 0;

    delete ui;
}

void MainWindowController::displayApp(const bool open)
{
    if (open) {
        show();
        raise();
    }
}

void MainWindowController::displayUpdate(
    const bool open,
    UpdateView *update)
{

}

void MainWindowController::displayOnlineState(
    const bool is_online)
{
}

void MainWindowController::displayUrl(
    const QUrl url)
{
    QDesktopServices::openUrl(url);
}

void MainWindowController::displayLogin(
        const bool open,
        const uint64_t user_id)
{
    if (open) {
        loginWidget->setVisible(true);
    }
    if (user_id) {
        loginWidget->setVisible(false);
    }
}

void MainWindowController::displayReminder(
    const QString title,
    const QString informative_text)
{

}

void MainWindowController::displayTimeEntryList(
    const bool open,
    QVector<TimeEntryView *> list)
{

}

void MainWindowController::displayTimeEntryEditor(
    const bool open,
    TimeEntryView *te,
    const QString focused_field_name)
{

}

void MainWindowController::displaySettings(
    const bool open,
    SettingsView *settings)
{

}

void MainWindowController::displayRunningTimerState(
    TimeEntryView *te)
{

}

void MainWindowController::displayStoppedTimerState()
{

}

void MainWindowController::displayIdleNotification(
    const QString since,
    const QString duration,
    const uint64_t started)
{

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

void MainWindowController::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    bool started = TogglApi::instance->startEvents();
    Q_ASSERT(started);
}
