// Copyright 2014 Toggl Desktop developers.

#include "./mainwindowcontroller.h"
#include "./ui_mainwindowcontroller.h"

#include <iostream>  // NOLINT

#include <QAction>  // NOLINT
#include <QCloseEvent>  // NOLINT
#include <QtConcurrent/QtConcurrent>  // NOLINT
#include <QDebug>  // NOLINT
#include <QDesktopServices>  // NOLINT
#include <QImageReader>  // NOLINT
#include <QLabel>  // NOLINT
#include <QMenu>  // NOLINT
#include <QMessageBox>  // NOLINT
#include <QSettings>  // NOLINT
#include <QVBoxLayout>  // NOLINT
#include <QStatusBar>  // NOLINT

#include "./toggl.h"
#include "./errorviewcontroller.h"
#include "./loginwidget.h"
#include "./timeentrylistwidget.h"
#include "./timeentryeditorwidget.h"

MainWindowController::MainWindowController(
    QWidget *parent,
    QString logPathOverride,
    QString dbPathOverride,
    QString scriptPath)
    : QMainWindow(parent),
  ui(new Ui::MainWindowController),
  togglApi(new TogglApi(0, logPathOverride, dbPathOverride)),
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
  preferencesDialog(new PreferencesDialog(this)),
  aboutDialog(new AboutDialog(this)),
  feedbackDialog(new FeedbackDialog(this)),
  idleNotificationDialog(new IdleNotificationDialog(this)),
  trayIcon(0),
  reminder(false),
  script(scriptPath) {
    TogglApi::instance->setEnvironment(APP_ENVIRONMENT);

    ui->setupUi(this);

    ui->menuBar->setVisible(true);

    QVBoxLayout *verticalLayout = new QVBoxLayout();
    verticalLayout->addWidget(new ErrorViewController());
    verticalLayout->addWidget(new LoginWidget());
    verticalLayout->addWidget(new TimeEntryListWidget());
    verticalLayout->addWidget(new TimeEntryEditorWidget());
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    verticalLayout->setSpacing(0);
    centralWidget()->setLayout(verticalLayout);

    readSettings();

    connect(TogglApi::instance, SIGNAL(displayApp(bool)),  // NOLINT
            this, SLOT(displayApp(bool)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayStoppedTimerState()),  // NOLINT
            this, SLOT(displayStoppedTimerState()));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayRunningTimerState(TimeEntryView*)),  // NOLINT
            this, SLOT(displayRunningTimerState(TimeEntryView*)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayReminder(QString,QString)),  // NOLINT
            this, SLOT(displayReminder(QString,QString)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayUpdate(QString)),  // NOLINT
            this, SLOT(displayUpdate(QString)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayOnlineState(int64_t)),  // NOLINT
            this, SLOT(displayOnlineState(int64_t)));  // NOLINT


    hasTrayIconCached = hasTrayIcon();
    if (hasTrayIconCached) {
        icon.addFile(QString::fromUtf8(":/icons/1024x1024/toggldesktop.png"));

        iconDisabled.addFile(QString::fromUtf8(
            ":/icons/1024x1024/toggldesktop_gray.png"));

        trayIcon = new QSystemTrayIcon(this);
    }

    connectMenuActions();
    enableMenuActions();

    if (hasTrayIconCached) {
        // icon is set in enableMenuActions based on if tracking is in progress
        trayIcon->show();
    } else {
        setWindowIcon(icon);
    }
}

MainWindowController::~MainWindowController() {
    delete togglApi;
    togglApi = 0;

    delete ui;
}

void MainWindowController::displayOnlineState(
    int64_t state) {

    switch (state) {
    case 0:  // online
        statusBar()->clearMessage();
        break;
    case 1:  // no network
        statusBar()->showMessage("Status: Offline, no network");
        break;
    case 2:  // backend down
        statusBar()->showMessage("Status: Offline, Toggl not responding");
        break;
    default:
        qDebug() << "Unknown online state " << state;
        break;
    }
}

void MainWindowController::displayReminder(
    const QString title,
    const QString informative_text) {

    if (reminder) {
        return;
    }
    reminder = true;

    QMessageBox(
        QMessageBox::Information,
        title,
        informative_text,
        QMessageBox::Ok).exec();

    reminder = false;
}

void MainWindowController::displayLogin(
    const bool open,
    const uint64_t user_id) {

    loggedIn = !open && user_id;
    enableMenuActions();
}

void MainWindowController::displayRunningTimerState(
    TimeEntryView *te) {
    tracking = true;
    enableMenuActions();
}

void MainWindowController::displayStoppedTimerState() {
    tracking = false;
    enableMenuActions();
}

void MainWindowController::enableMenuActions() {
    actionNew->setEnabled(loggedIn);
    actionContinue->setEnabled(loggedIn && !tracking);
    actionStop->setEnabled(loggedIn && tracking);
    actionSync->setEnabled(loggedIn);
    actionLogout->setEnabled(loggedIn);
    actionClear_Cache->setEnabled(loggedIn);
    actionSend_Feedback->setEnabled(loggedIn);
    actionReports->setEnabled(loggedIn);
    if (hasTrayIconCached) {
        if (tracking) {
            trayIcon->setIcon(icon);
            setWindowIcon(icon);
        } else {
            trayIcon->setIcon(iconDisabled);
            setWindowIcon(iconDisabled);
        }
    }
}

void MainWindowController::connectMenuActions() {
    foreach(QMenu *menu, ui->menuBar->findChildren<QMenu *>()) {
        if (trayIcon) {
            trayIcon->setContextMenu(menu);
        }
        foreach(QAction *action, menu->actions()) {
            connectMenuAction(action);
        }
    }
}

void MainWindowController::connectMenuAction(
    QAction *action) {
    if ("actionNew" == action->objectName()) {
        actionNew = action;
        connect(action, SIGNAL(triggered()), this, SLOT(onActionNew()));
    } else if ("actionContinue" == action->objectName()) {
        actionContinue = action;
        connect(action, SIGNAL(triggered()), this, SLOT(onActionContinue()));
    } else if ("actionStop" == action->objectName()) {
        actionStop = action;
        connect(action, SIGNAL(triggered()), this, SLOT(onActionStop()));
    } else if ("actionSync" == action->objectName()) {
        actionSync = action;
        connect(action, SIGNAL(triggered()), this, SLOT(onActionSync()));
    } else if ("actionLogout" == action->objectName()) {
        actionLogout = action;
        connect(action, SIGNAL(triggered()), this, SLOT(onActionLogout()));
    } else if ("actionClear_Cache" == action->objectName()) {
        actionClear_Cache = action;
        connect(action, SIGNAL(triggered()), this, SLOT(onActionClear_Cache()));
    } else if ("actionSend_Feedback" == action->objectName()) {
        actionSend_Feedback = action;
        connect(action, SIGNAL(triggered()),
                this, SLOT(onActionSend_Feedback()));
    } else if ("actionReports" == action->objectName()) {
        actionReports = action;
        connect(action, SIGNAL(triggered()), this, SLOT(onActionReports()));
    } else if ("actionShow" == action->objectName()) {
        connect(action, SIGNAL(triggered()), this, SLOT(onActionShow()));
    } else if ("actionPreferences" == action->objectName()) {
        connect(action, SIGNAL(triggered()), this, SLOT(onActionPreferences()));
    } else if ("actionAbout" == action->objectName()) {
        connect(action, SIGNAL(triggered()), this, SLOT(onActionAbout()));
    } else if ("actionQuit" == action->objectName()) {
        connect(action, SIGNAL(triggered()), this, SLOT(onActionQuit()));
    } else if ("actionHelp" == action->objectName()) {
        connect(action, SIGNAL(triggered()), this, SLOT(onActionHelp()));
    }
}

void MainWindowController::onActionNew() {
    TogglApi::instance->start("", "", 0, 0);
}

void MainWindowController::onActionContinue() {
    TogglApi::instance->continueLatestTimeEntry();
}

void MainWindowController::onActionStop() {
    TogglApi::instance->stop();
}

void MainWindowController::onActionShow() {
    displayApp(true);
}

void MainWindowController::onActionSync() {
    TogglApi::instance->sync();
}

void MainWindowController::onActionReports() {
    TogglApi::instance->openInBrowser();
}

void MainWindowController::onActionPreferences() {
    TogglApi::instance->editPreferences();
}

void MainWindowController::onActionAbout() {
    aboutDialog->show();
}

void MainWindowController::onActionSend_Feedback() {
    feedbackDialog->show();
}

void MainWindowController::onActionLogout() {
    TogglApi::instance->logout();
}

void MainWindowController::onActionQuit() {
    quitApp();
}

void MainWindowController::quitApp() {
    TogglApi::instance->shutdown = true;
    qApp->exit(0);
}

void MainWindowController::onActionClear_Cache() {
    if (QMessageBox::Ok == QMessageBox(
        QMessageBox::Question,
        "Clear Cache?",
        "Clearing cache will delete any unsaved time entries and log you out.",
        QMessageBox::Ok|QMessageBox::Cancel).exec()) {
        TogglApi::instance->clearCache();
    }
}

void MainWindowController::onActionHelp() {
    TogglApi::instance->getSupport();
}

void MainWindowController::displayApp(const bool open) {
    if (open) {
        show();
        raise();
    }
}

void MainWindowController::readSettings() {
    QSettings settings("Toggl", "TogglDesktop");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindowController::writeSettings() {
    QSettings settings("Toggl", "TogglDesktop");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

void MainWindowController::closeEvent(QCloseEvent *event) {
    QMessageBox::StandardButton dialog;
    dialog = QMessageBox::question(this,
                                   "Toggl Desktop",
                                   "Really quit the app?",
                                   QMessageBox::Ok | QMessageBox::Cancel);
    if (QMessageBox::Ok == dialog) {
        writeSettings();
        close();
    } else {
        event->ignore();
    }

    QMainWindow::closeEvent(event);
}

bool MainWindowController::hasTrayIcon() const {
    QString currentDesktop = QProcessEnvironment::systemEnvironment().value(
        "XDG_CURRENT_DESKTOP", "");
    return "Unity" != currentDesktop;
}

void MainWindowController::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    if (!TogglApi::instance->startEvents()) {
        QMessageBox(
            QMessageBox::Warning,
            "Error",
            "The application could not start. Please inspect the log file.",
            QMessageBox::Ok|QMessageBox::Cancel).exec();
        return;
    }
    if (script.isEmpty()) {
        qDebug() << "no script to run";
        return;
    }
    qDebug() << "will run script: " << script;

    QtConcurrent::run(this, &MainWindowController::runScript);
}

void MainWindowController::displayUpdate(const QString url) {
    if (aboutDialog->isVisible()
            || url.isEmpty()) {
        return;
    }
    if (QMessageBox::Yes == QMessageBox(
        QMessageBox::Question,
        "Download new version?",
        "A new version of Toggl Desktop is available. Continue with download?",
        QMessageBox::No|QMessageBox::Yes).exec()) {
        QDesktopServices::openUrl(QUrl(url));
        quitApp();
    }
}

void MainWindowController::runScript() {
    if (TogglApi::instance->runScriptFile(script)) {
        quitApp();
    }
}
