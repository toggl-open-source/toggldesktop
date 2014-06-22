#include "mainwindowcontroller.h"
#include "ui_mainwindowcontroller.h"

#include <iostream>

#include <QSettings>
#include <QCloseEvent>
#include <QMessageBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>
#include <QAction>
#include <QMenu>

#include "toggl_api.h"
#include "errorviewcontroller.h"
#include "loginwidget.h"
#include "timeentrylistwidget.h"
#include "timeentryeditorwidget.h"
#include "preferencesdialog.h"

MainWindowController::MainWindowController(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowController),
    shutdown(false),
    togglApi(new TogglApi()),
    tracking(false),
    loggedIn(false),
    actionNew(0),
    actionContinue(0),
    actionStop(0),
    actionSync(0),
    actionLogout(0),
    actionClear_Cache(0),
    actionSend_Feedback(0),
    actionReports(0),
    preferencesDialog(new PreferencesDialog(this))
{
    ui->setupUi(this);

    ui->menuBar->setVisible(true);

    QVBoxLayout *verticalLayout = new QVBoxLayout();
    verticalLayout->addWidget(new ErrorViewController());
    verticalLayout->addWidget(new LoginWidget());
    verticalLayout->addWidget(new TimeEntryListWidget());
    verticalLayout->addWidget(new TimeEntryEditorWidget());
    centralWidget()->setLayout(verticalLayout);

    readSettings();

    connect(TogglApi::instance, SIGNAL(displayApp(bool)), this, SLOT(displayApp(bool)));

    connect(TogglApi::instance, SIGNAL(displayStoppedTimerState()),
            this, SLOT(displayStoppedTimerState()));

    connect(TogglApi::instance, SIGNAL(displayRunningTimerState(TimeEntryView*)),
            this, SLOT(displayRunningTimerState(TimeEntryView*)));

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),
            this, SLOT(displayLogin(bool,uint64_t)));

    connectMenuActions();
    enableMenuActions();
}

MainWindowController::~MainWindowController()
{
    delete togglApi;
    togglApi = 0;

    delete ui;
}

void MainWindowController::displayLogin(
    const bool open,
    const uint64_t user_id) {

    loggedIn = !open && user_id;
    enableMenuActions();
}

void MainWindowController::displayRunningTimerState(
    TimeEntryView *te)
{
    tracking = true;
    enableMenuActions();
}

void MainWindowController::displayStoppedTimerState()
{
    tracking = false;
    enableMenuActions();
}

void MainWindowController::enableMenuActions()
{
    actionNew->setEnabled(loggedIn);
    actionContinue->setEnabled(loggedIn && !tracking);
    actionStop->setEnabled(loggedIn && tracking);
    actionSync->setEnabled(loggedIn);
    actionLogout->setEnabled(loggedIn);
    actionClear_Cache->setEnabled(loggedIn);
    actionSend_Feedback->setEnabled(loggedIn);
    actionReports->setEnabled(loggedIn);
}

void MainWindowController::connectMenuActions()
{
    foreach(QMenu *menu, ui->menuBar->findChildren<QMenu *>())
    {
        foreach(QAction *action, menu->actions())
        {
            if ("actionNew" == action->objectName())
            {
                actionNew = action;
                connect(action, SIGNAL(triggered()), this, SLOT(onActionNew()));
            }
            else if ("actionContinue" == action->objectName())
            {
                actionContinue = action;
                connect(action, SIGNAL(triggered()), this, SLOT(onActionContinue()));
            }
            else if ("actionStop" == action->objectName())
            {
                actionStop = action;
                connect(action, SIGNAL(triggered()), this, SLOT(onActionStop()));
            }
            else if ("actionSync" == action->objectName())
            {
                actionSync = action;
                connect(action, SIGNAL(triggered()), this, SLOT(onActionSync()));
            }
            else if ("actionLogout" == action->objectName())
            {
                actionLogout = action;
                connect(action, SIGNAL(triggered()), this, SLOT(onActionLogout()));
            }
            else if ("actionClear_Cache" == action->objectName())
            {
                actionClear_Cache = action;
                connect(action, SIGNAL(triggered()), this, SLOT(onActionClear_Cache()));
            }
            else if ("actionSend_Feedback" == action->objectName())
            {
                actionSend_Feedback = action;
                connect(action, SIGNAL(triggered()), this, SLOT(onActionSend_Feedback()));
            }
            else if ("actionReports" == action->objectName())
            {
                actionReports = action;
                connect(action, SIGNAL(triggered()), this, SLOT(onActionReports()));
            }
            else if ("actionShow" == action->objectName())
            {
                connect(action, SIGNAL(triggered()), this, SLOT(onActionShow()));
            }
            else if ("actionPreferences" == action->objectName())
            {
                connect(action, SIGNAL(triggered()), this, SLOT(onActionPreferences()));
            }
            else if ("actionAbout" == action->objectName())
            {
                connect(action, SIGNAL(triggered()), this, SLOT(onActionAbout()));
            }
            else if ("actionQuit" == action->objectName())
            {
                connect(action, SIGNAL(triggered()), this, SLOT(onActionQuit()));
            }
            else if ("actionHelp" == action->objectName())
            {
                connect(action, SIGNAL(triggered()), this, SLOT(onActionHelp()));
            }
        }
    }
}

void MainWindowController::onActionNew()
{
    TogglApi::instance->start("", "", 0, 0);
}

void MainWindowController::onActionContinue()
{
    TogglApi::instance->continueLatestTimeEntry();
}

void MainWindowController::onActionStop()
{
    TogglApi::instance->stop();
}

void MainWindowController::onActionShow()
{
    displayApp(true);
}

void MainWindowController::onActionSync()
{
    TogglApi::instance->sync();
}

void MainWindowController::onActionReports()
{
    TogglApi::instance->openInBrowser();
}

void MainWindowController::onActionPreferences()
{
    TogglApi::instance->editPreferences();
}

void MainWindowController::onActionAbout()
{
    TogglApi::instance->about();
}

void MainWindowController::onActionSend_Feedback()
{
    // FIXME: open feedback dialog
}

void MainWindowController::onActionLogout()
{
    TogglApi::instance->logout();
}

void MainWindowController::onActionQuit()
{
    shutdown = true;
    qApp->exit(0);
}

void MainWindowController::onActionClear_Cache()
{
    if (QMessageBox::Ok == QMessageBox(QMessageBox::Question,
                                        "Clear Cache?",
                                        "Clearing cache will delete any unsaved time entries and log you out.",
                                        QMessageBox::Ok|QMessageBox::Cancel).exec())
    {
        TogglApi::instance->clearCache();
    }
}

void MainWindowController::onActionHelp()
{
    TogglApi::instance->getSupport();
}

void MainWindowController::displayApp(const bool open)
{
    if (open) {
        show();
        raise();
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
    if (!shutdown) {
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
    if (!started)
    {
        QMessageBox(QMessageBox::Warning,
            "Error",
            "The application could not start. Please inspect the log file. Sorry :S",
            QMessageBox::Ok|QMessageBox::Cancel).exec();
    }
}
