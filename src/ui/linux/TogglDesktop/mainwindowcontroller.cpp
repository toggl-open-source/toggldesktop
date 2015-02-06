// Copyright 2014 Toggl Desktop developers.

#include "./mainwindowcontroller.h"
#include "./ui_mainwindowcontroller.h"

#include <iostream>  // NOLINT

#include <QSettings>  // NOLINT
#include <QCloseEvent>  // NOLINT
#include <QMessageBox>  // NOLINT
#include <QLabel>  // NOLINT
#include <QVBoxLayout>  // NOLINT
#include <QDebug>  // NOLINT
#include <QDesktopServices>  // NOLINT
#include <QAction>  // NOLINT
#include <QMenu>  // NOLINT
#include <QImageReader>  // NOLINT

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

    connect(TogglApi::instance, SIGNAL(displayUpdate(bool,UpdateView*)),  // NOLINT
            this, SLOT(displayUpdate(bool,UpdateView*)));  // NOLINT

    icon.addFile(QString::fromUtf8(":/icons/1024x1024/toggldesktop.png"));
    setWindowIcon(icon);

    connectMenuActions();
    enableMenuActions();
}

MainWindowController::~MainWindowController() {
    delete togglApi;
    togglApi = 0;

    delete ui;
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
}

void MainWindowController::connectMenuActions() {
    foreach(QMenu *menu, ui->menuBar->findChildren<QMenu *>()) {
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
    TogglApi::instance->about();
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
}

void MainWindowController::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    if (!TogglApi::instance->startEvents()) {
        QMessageBox(
		QMessageBox::Warning,
		"Error",
		"The application could not start. Please inspect the log file. Sorry!",
		QMessageBox::Ok|QMessageBox::Cancel).exec();
	return;
    }
    runScript();
}

void MainWindowController::displayUpdate(const bool open, UpdateView *view) {
    if (open || aboutDialog->isVisible()
            || view->IsChecking || !view->IsUpdateAvailable) {
        return;
    }
    if (QMessageBox::Yes == QMessageBox(
        QMessageBox::Question,
        "Download new version?",
        "A new version of Toggl Desktop is available (" + view->Version + ")."
        + " Continue with download?",
        QMessageBox::No|QMessageBox::Yes).exec()) {
        QDesktopServices::openUrl(QUrl(view->URL));
        quitApp();
    }
}

void MainWindowController::runScript() {
	if (script.isEmpty()) {
		return;
	}
	// FIXME: load and execute Lua script
}
