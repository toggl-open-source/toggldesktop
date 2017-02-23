// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_MAINWINDOWCONTROLLER_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_MAINWINDOWCONTROLLER_H_

#include <QMainWindow>
#include <QSystemTrayIcon>

#include <stdbool.h>
#include <stdint.h>

#include "./toggl.h"
#include "./loginwidget.h"
#include "./timeentrylistwidget.h"
#include "./preferencesdialog.h"
#include "./aboutdialog.h"
#include "./feedbackdialog.h"
#include "./idlenotificationdialog.h"
#include "./qxtglobalshortcut.h"

namespace Ui {
class MainWindowController;
}

class MainWindowController : public QMainWindow {
    Q_OBJECT

 public:
    MainWindowController(
        QWidget *parent = 0,
        QString logPathOverride = "",
        QString dbPathOverride = "",
        QString scriptPath = "");
    ~MainWindowController();

    static MainWindowController *Instance;

    void quitApp();
    void setShortcuts();

 protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);
    void runScript();

 private slots:  // NOLINT
    void toggleWindow(QSystemTrayIcon::ActivationReason r);

    void displayApp(const bool open);

    void displayRunningTimerState(TimeEntryView *te);

    void displayStoppedTimerState();

    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void displayPomodoro(
        const QString title,
        const QString informative_text);

    void displayPomodoroBreak(
        const QString title,
        const QString informative_text);

    void displayReminder(
        const QString title,
        const QString informative_text);

    void displayUpdate(const QString url);

    void displayOnlineState(int64_t);
    void showHideHotkeyPressed();
    void continueStopHotkeyPressed();

    void onActionNew();
    void onActionContinue();
    void onActionStop();
    void onActionShow();
    void onActionSync();
    void onActionReports();
    void onActionPreferences();
    void onActionAbout();
    void onActionSend_Feedback();
    void onActionLogout();
    void onActionQuit();
    void onActionClear_Cache();
    void onActionHelp();

    void updateShowHideShortcut();
    void updateContinueStopShortcut();

 private:
    Ui::MainWindowController *ui;

    TogglApi *togglApi;

    bool tracking;
    bool loggedIn;
    bool startInBackground;

    QxtGlobalShortcut* showHide;
    QxtGlobalShortcut* continueStop;

    QAction *actionEmail;
    QAction *actionNew;
    QAction *actionContinue;
    QAction *actionStop;
    QAction *actionSync;
    QAction *actionLogout;
    QAction *actionClear_Cache;
    QAction *actionSend_Feedback;
    QAction *actionReports;

    PreferencesDialog *preferencesDialog;
    AboutDialog *aboutDialog;
    FeedbackDialog *feedbackDialog;
    IdleNotificationDialog *idleNotificationDialog;

    QIcon icon;
    QIcon iconDisabled;
    QSystemTrayIcon *trayIcon;

    bool pomodoro;
    bool reminder;

    QString script;

    void readSettings();
    void writeSettings();

    void connectMenuActions();
    void connectMenuAction(QAction *action);
    void enableMenuActions();

    bool hasTrayIconCached;

    bool hasTrayIcon() const;

    bool ui_started;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_MAINWINDOWCONTROLLER_H_
