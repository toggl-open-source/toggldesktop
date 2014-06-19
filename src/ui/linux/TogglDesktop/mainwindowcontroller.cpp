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
#include "timeentrylistwidget.h"

MainWindowController::MainWindowController(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowController),
    ctx_(0),
    shutdown_(false),
    togglApi(new TogglApi())
{
    ui->setupUi(this);

    ui->mainToolBar->setVisible(false);

    QVBoxLayout *verticalLayout = new QVBoxLayout();
    verticalLayout->addWidget(new ErrorViewController());
    verticalLayout->addWidget(new LoginWidget());
    verticalLayout->addWidget(new TimeEntryListWidget());;
    centralWidget()->setLayout(verticalLayout);

    readSettings();

    connect(TogglApi::instance, SIGNAL(displayApp(bool)), this, SLOT(displayApp(bool)));
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
