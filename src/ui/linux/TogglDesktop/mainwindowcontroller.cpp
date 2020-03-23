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
#include <QStackedWidget>  // NOLINT
#include <QStatusBar>  // NOLINT
#include <QPushButton>  // NOLINT

#include "./toggl.h"
#include "./timeentryeditorwidget.h"
#include "./timeentrylistwidget.h"
#include "./timerwidget.h"
#include "./errorviewcontroller.h"
#include "./timeentrycellwidget.h"

MainWindowController::MainWindowController(
    QWidget *parent,
    QString logPathOverride,
    QString dbPathOverride,
    QString scriptPath)
    : QMainWindow(parent),
  ui(new Ui::MainWindowController),
  togglApi(new TogglApi(nullptr, logPathOverride, dbPathOverride)),
  tracking(false),
  loggedIn(false),
  preferencesDialog(new PreferencesDialog(this)),
  aboutDialog(new AboutDialog(this)),
  feedbackDialog(new FeedbackDialog(this)),
  icon(":/icons/1024x1024/toggldesktop.png"),
  iconDisabled(":/icons/1024x1024/toggldesktop_gray.png"),
  trayIcon(nullptr),
  pomodoro(false),
  script(scriptPath),
  powerManagement(new PowerManagement(this)),
  networkManagement(new NetworkManagement(this)),
  shortcutDelete(QKeySequence(Qt::CTRL + Qt::Key_Delete), this),
  shortcutPause(QKeySequence(Qt::CTRL + Qt::Key_Space), this),
  shortcutConfirm(QKeySequence(Qt::CTRL + Qt::Key_Return), this),
  shortcutGroupOpen(QKeySequence(Qt::Key_Right), this),
  shortcutGroupClose(QKeySequence(Qt::Key_Left), this),
  ui_started(false) {
    ui->setupUi(this);

    ui->menuBar->setVisible(true);

    readSettings();

    connect(TogglApi::instance, SIGNAL(displayApp(bool)),  // NOLINT
            this, SLOT(displayApp(bool)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayStoppedTimerState()),  // NOLINT
            this, SLOT(displayStoppedTimerState()));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayRunningTimerState(TimeEntryView*)),  // NOLINT
            this, SLOT(displayRunningTimerState(TimeEntryView*)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayPomodoro(QString,QString)),  // NOLINT
            this, SLOT(displayPomodoro(QString,QString)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayPomodoroBreak(QString,QString)),  // NOLINT
            this, SLOT(displayPomodoroBreak(QString,QString)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayUpdate(QString)),  // NOLINT
            this, SLOT(displayUpdate(QString)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayOnlineState(int64_t)),  // NOLINT
            this, SLOT(displayOnlineState(int64_t)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(updateShowHideShortcut()),  // NOLINT
            this, SLOT(updateShowHideShortcut()));  // NOLINT

    connect(TogglApi::instance, SIGNAL(updateContinueStopShortcut()),  // NOLINT
            this, SLOT(updateContinueStopShortcut()));  // NOLINT

    setWindowIcon(icon);
    trayIcon = new SystemTray(this, icon);

    setShortcuts();
    connectMenuActions();
    enableMenuActions();

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(toggleWindow(QSystemTrayIcon::ActivationReason)));
    connect(networkManagement, &NetworkManagement::onlineStateChanged,
            this, &MainWindowController::onOnlineStateChanged);
    onOnlineStateChanged();
    restoreLastWindowsFrame();
}

MainWindowController::~MainWindowController() {
    delete togglApi;
    togglApi = nullptr;

    delete ui;
}

void MainWindowController::toggleWindow(QSystemTrayIcon::ActivationReason r) {
    if (r == QSystemTrayIcon::Trigger) {
        if (!this->isVisible()) {
            this->show();
        } else {
            this->hide();
        }
    }
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

void MainWindowController::displayPomodoro(
    const QString title,
    const QString informative_text) {

    if (pomodoro) {
        return;
    }
    pomodoro = true;

    QMessageBox msgBox;
    msgBox.setWindowTitle("Pomodoro Timer");
    msgBox.setText(title);
    msgBox.setInformativeText(informative_text);
    QPushButton *continueButton =
        msgBox.addButton(tr("Continue"), QMessageBox::YesRole);
    QPushButton *closeButton =
        msgBox.addButton(tr("Close"), QMessageBox::NoRole);
    msgBox.setDefaultButton(closeButton);
    msgBox.setEscapeButton(closeButton);

    msgBox.exec();

    if (msgBox.clickedButton() == continueButton) {
        TogglApi::instance->continueLatestTimeEntry();
    }

    pomodoro = false;
}

void MainWindowController::displayPomodoroBreak(
    const QString title,
    const QString informative_text) {

    if (pomodoro) {
        return;
    }
    pomodoro = true;

    QMessageBox msgBox;
    msgBox.setWindowTitle("Pomodoro Break Timer");
    msgBox.setText(title);
    msgBox.setInformativeText(informative_text);
    QPushButton *continueButton =
        msgBox.addButton(tr("Continue"), QMessageBox::YesRole);
    QPushButton *closeButton =
        msgBox.addButton(tr("Close"), QMessageBox::NoRole);
    msgBox.setDefaultButton(closeButton);
    msgBox.setEscapeButton(closeButton);

    msgBox.exec();

    if (msgBox.clickedButton() == continueButton) {
        TogglApi::instance->continueLatestTimeEntry();
    }

    pomodoro = false;
}

void MainWindowController::displayLogin(
    const bool open,
    const uint64_t user_id) {

    loggedIn = !open && user_id;
    enableMenuActions();
}

void MainWindowController::displayRunningTimerState(
    TimeEntryView *te) {
    Q_UNUSED(te);
    tracking = true;
    enableMenuActions();
}

void MainWindowController::displayStoppedTimerState() {
    tracking = false;
    enableMenuActions();
}

void MainWindowController::enableMenuActions() {
    ui->actionNew->setEnabled(loggedIn);
    ui->actionContinue->setEnabled(loggedIn && !tracking);
    ui->actionStop->setEnabled(loggedIn && tracking);
    ui->actionSync->setEnabled(loggedIn);
    ui->actionLogout->setEnabled(loggedIn);
    ui->actionClear_Cache->setEnabled(loggedIn);
    ui->actionSend_Feedback->setEnabled(loggedIn);
    ui->actionReports->setEnabled(loggedIn);
    ui->actionEmail->setText(TogglApi::instance->userEmail());
    if (tracking) {
        setWindowIcon(icon);
        trayIcon->setIcon(icon);
    } else {
        setWindowIcon(iconDisabled);
        trayIcon->setIcon(iconDisabled);
    }
}

void MainWindowController::showHideHotkeyPressed() {
    if (this->isVisible()) {
        if (this->isActiveWindow()) {
            hide();
        } else {
            activateWindow();
        }
    } else {
        onActionShow();
        activateWindow();
    }
}

void MainWindowController::continueStopHotkeyPressed() {
    if (tracking) {
        onActionStop();
    } else {
        onActionContinue();
    }
}

void MainWindowController::updateShowHideShortcut() {
    showHide->setShortcut(
        QKeySequence(TogglApi::instance->getShowHideKey()));
}

void MainWindowController::updateContinueStopShortcut() {
    continueStop->setShortcut(
        QKeySequence(TogglApi::instance->getContinueStopKey()));
}

void MainWindowController::onOnlineStateChanged() {
    if (networkManagement->isOnline()) {
        TogglApi::instance->setOnline();
    }
}

void MainWindowController::onShortcutDelete() {
    if (ui->stackedWidget->currentWidget() == ui->timeEntryListWidget) {
        if (ui->timeEntryListWidget->focusWidget() &&
                ui->timeEntryListWidget->focusWidget()->parentWidget() &&
                qobject_cast<TimeEntryCellWidget*>(ui->timeEntryListWidget->focusWidget()->parentWidget())) {
            qobject_cast<TimeEntryCellWidget*>(ui->timeEntryListWidget->focusWidget()->parentWidget())->deleteTimeEntry();
        }
        else {
            ui->timeEntryListWidget->timer()->deleteTimeEntry();
        }
    }
    else if (ui->stackedWidget->currentWidget() == ui->timeEntryEditorWidget) {
        ui->timeEntryEditorWidget->deleteTimeEntry();
    }
}

void MainWindowController::onShortcutPause() {
    auto w = focusWidget();
    while (w) {
        auto timer = qobject_cast<TimerWidget*>(w);
        auto timeEntryList = qobject_cast<TimeEntryListWidget*>(w);
        auto timeEntryEdit = qobject_cast<TimeEntryEditorWidget*>(w);
        if (timer) {
            continueStopHotkeyPressed();
            return;
        }
        else if (timeEntryList) {
            bool thisItem = !timeEntryList->timer()->currentEntryGuid().isEmpty() &&
                            timeEntryList->highlightedCell() &&
                            timeEntryList->timer()->currentEntryGuid() == timeEntryList->highlightedCell()->entryGuid();
            QString selectedGuid = timeEntryList->highlightedCell() ? timeEntryList->highlightedCell()->entryGuid() : QString();
            if (tracking) {
                onActionStop();
            }
            if (!thisItem) {
                TogglApi::instance->continueTimeEntry(selectedGuid);
            }
            return;
        }
        else if (timeEntryEdit) {
            return;
        }
        w = w->parentWidget();
    }
}

void MainWindowController::onShortcutConfirm() {
    auto w = focusWidget();
    while (w) {
        auto timer = qobject_cast<TimerWidget*>(w);
        auto timeEntryList = qobject_cast<TimeEntryListWidget*>(w);
        auto timeEntryEdit = qobject_cast<TimeEntryEditorWidget*>(w);
        if (timer) {
            TogglApi::instance->editRunningTimeEntry("");
            return;
        }
        else if (timeEntryList) {
            QString selectedGuid = timeEntryList->highlightedCell() ? timeEntryList->highlightedCell()->entryGuid() : QString();
            TogglApi::instance->editTimeEntry(selectedGuid, "description");
            return;
        }
        else if (timeEntryEdit) {
            timeEntryEdit->clickDone();
            return;
        }
        w = w->parentWidget();
    }
}

void MainWindowController::onShortcutGroupOpen() {
    if (ui->stackedWidget->currentWidget() == ui->timeEntryListWidget) {
        if (ui->timeEntryListWidget->focusWidget() &&
                ui->timeEntryListWidget->focusWidget()->parentWidget() &&
                qobject_cast<TimeEntryCellWidget*>(ui->timeEntryListWidget->focusWidget()->parentWidget())) {
            qobject_cast<TimeEntryCellWidget*>(ui->timeEntryListWidget->focusWidget()->parentWidget())->toggleGroup(true);
        }
    }
}

void MainWindowController::onShortcutGroupClose() {
    if (ui->stackedWidget->currentWidget() == ui->timeEntryListWidget) {
        if (ui->timeEntryListWidget->focusWidget() &&
                ui->timeEntryListWidget->focusWidget()->parentWidget() &&
                qobject_cast<TimeEntryCellWidget*>(ui->timeEntryListWidget->focusWidget()->parentWidget())) {
            qobject_cast<TimeEntryCellWidget*>(ui->timeEntryListWidget->focusWidget()->parentWidget())->toggleGroup(false);
        }
    }
}

void MainWindowController::setShortcuts() {
    showHide = new QxtGlobalShortcut(this);
    connect(showHide, SIGNAL(activated()),
            this, SLOT(showHideHotkeyPressed()));

    updateShowHideShortcut();

    continueStop = new QxtGlobalShortcut(this);
    connect(continueStop, SIGNAL(activated()),
            this, SLOT(continueStopHotkeyPressed()));

    updateContinueStopShortcut();

    connect(&shortcutDelete, &QShortcut::activated,
            this, &MainWindowController::onShortcutDelete);
    connect(&shortcutPause, &QShortcut::activated,
            this, &MainWindowController::onShortcutPause);
    connect(&shortcutConfirm, &QShortcut::activated,
            this, &MainWindowController::onShortcutConfirm);
    connect(&shortcutGroupOpen, &QShortcut::activated,
            this, &MainWindowController::onShortcutGroupOpen);
    connect(&shortcutGroupClose, &QShortcut::activated,
            this, &MainWindowController::onShortcutGroupClose);
}

void MainWindowController::connectMenuActions() {
    connect(ui->actionNew, &QAction::triggered, this, &MainWindowController::onActionNew);
    connect(ui->actionContinue,  &QAction::triggered, this, &MainWindowController::onActionContinue);
    connect(ui->actionStop,  &QAction::triggered, this, &MainWindowController::onActionStop);
    connect(ui->actionSync,  &QAction::triggered, this, &MainWindowController::onActionSync);
    connect(ui->actionLogout,  &QAction::triggered, this, &MainWindowController::onActionLogout);
    connect(ui->actionClear_Cache,  &QAction::triggered, this, &MainWindowController::onActionClear_Cache);
    connect(ui->actionSend_Feedback,  &QAction::triggered, this, &MainWindowController::onActionSend_Feedback);
    connect(ui->actionReports,  &QAction::triggered, this, &MainWindowController::onActionReports);
    connect(ui->actionShow,  &QAction::triggered, this, &MainWindowController::onActionShow);
    connect(ui->actionPreferences,  &QAction::triggered, this, &MainWindowController::onActionPreferences);
    connect(ui->actionAbout,  &QAction::triggered, this, &MainWindowController::onActionAbout);
    connect(ui->actionQuit,  &QAction::triggered, this, &MainWindowController::onActionQuit);
    connect(ui->actionHelp,  &QAction::triggered, this, &MainWindowController::onActionHelp);

    QMenu *trayMenu = new QMenu(this);
for (auto act : ui->menuToggl_Desktop->actions()) {
        trayMenu->addAction(act);
    }

    trayIcon->setContextMenu(trayMenu);
}

void MainWindowController::onActionNew() {
    TogglApi::instance->start("", "", 0, 0, nullptr, false);
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
    TogglApi::instance->clear();
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

void MainWindowController::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);

    // Avoid 'user already logged in' error from double UI start
    if (ui_started) {
        return;
    }
    ui_started = true;

    if (!TogglApi::instance->startEvents()) {
        QMessageBox(
            QMessageBox::Warning,
            "Error",
            "The application could not start. Please inspect the log file.",
            QMessageBox::Ok|QMessageBox::Cancel).exec();
        return;
    }
    if (script.isEmpty()) {
        // qDebug() << "no script to run";
        return;
    }
    qDebug() << "will run script: " << script;
}

void MainWindowController::closeEvent(QCloseEvent *event) {

    // Window manager has requested the app to quit so just quit
    if (powerManagement->aboutToShutdown()) {
        QMainWindow::closeEvent(event);
        return;
    }

    // Save current windows frame
    TogglApi::instance->setWindowsFrameSetting(QRect(pos().x(),
            pos().y(),
            size().width(),
            size().height()));

    if (trayIcon->isVisible()) {
        event->ignore();
        hide();
        return;
    }

    QMessageBox::StandardButton dialog;
    dialog = QMessageBox::question(this,
                                   "Quit Toggl Desktop",
                                   "Really quit the app?",
                                   QMessageBox::Ok | QMessageBox::Cancel);
    if (QMessageBox::Ok == dialog) {
        writeSettings();
        close();
    } else {
        event->ignore();
        return;
    }

    QMainWindow::closeEvent(event);
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

void MainWindowController::restoreLastWindowsFrame() {
    const QRect frame = TogglApi::instance->getWindowsFrameSetting();
    move(frame.x(), frame.y());
    resize(frame.width(), frame.height());
}
