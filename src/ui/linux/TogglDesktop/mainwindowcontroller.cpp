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

MainWindowController::MainWindowController(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowController),
    shutdown(false),
    togglApi(new TogglApi()),
    menuActions(new QActionGroup(this))
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

    connectMenuActions();
}

MainWindowController::~MainWindowController()
{
    delete togglApi;
    togglApi = 0;

    delete ui;
}

void MainWindowController::connectMenuActions()
{
    foreach(QMenu *menu, ui->menuBar->findChildren<QMenu *>())
    {
        foreach(QAction *action, menu->actions())
        {
            menuActions->addAction(action);
        }
    }
    connect(menuActions, SIGNAL(triggered(QAction*)),
            this, SLOT(onAction(QAction*)));
}

void MainWindowController::onAction(QAction *action)
{
    if (action->objectName() == "actionNew")
    {
        TogglApi::instance->start("", "", 0, 0);
    }
    else if (action->objectName() == "actionContinue")
    {
        TogglApi::instance->continueLatestTimeEntry();
    }
    else if (action->objectName() == "actionStop")
    {
        TogglApi::instance->stop();
    }
    else if (action->objectName() == "actionShow")
    {
        displayApp(true);
    }
    else if (action->objectName() == "actionSync")
    {
        TogglApi::instance->sync();
    }
    else if (action->objectName() == "actionReports")
    {
        TogglApi::instance->openInBrowser();
    }
    else if (action->objectName() == "actionPreferences")
    {
        TogglApi::instance->editPreferences();
    }
    else if (action->objectName() == "actionAbout")
    {
        TogglApi::instance->about();
    }
    else if (action->objectName() == "actionSend_Feedback")
    {
        // FIXME: display feedback dialog
    }
    else if (action->objectName() == "actionLogout")
    {
        TogglApi::instance->logout();
    }
    else if (action->objectName() == "actionQuit")
    {
        shutdown = true;
        qApp->exit(0);
    }
    else if (action->objectName() == "actionClear_Cache")
    {
        if (QMessageBox::Ok == QMessageBox(QMessageBox::Question,
                                            "Clear Cache?",
                                            "Clearing cache will delete any unsaved time entries and log you out.",
                                            QMessageBox::Ok|QMessageBox::Cancel).exec())
        {
            TogglApi::instance->clearCache();

        }
    }
    else if (action->objectName() == "actionHelp")
    {
        TogglApi::instance->getSupport();
    }
    else
    {
        qDebug() << "unknown action " << action->objectName();
        Q_ASSERT(false);
    }
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
