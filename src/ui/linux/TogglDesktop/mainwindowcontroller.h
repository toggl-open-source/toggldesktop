#ifndef MAINWINDOWCONTROLLER_H
#define MAINWINDOWCONTROLLER_H

#include <QMainWindow>

#include <stdbool.h>
#include <stdint.h>

#include "toggl_api.h"
#include "loginwidget.h"
#include "timeentrylistwidget.h"
#include "preferencesdialog.h"
#include "aboutdialog.h"
#include "feedbackdialog.h"
#include "idlenotificationdialog.h"

namespace Ui {
class MainWindowController;
}

class MainWindowController : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindowController(QWidget *parent = 0);
    ~MainWindowController();

    static MainWindowController *Instance;

protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void displayApp(const bool open);
    void displayRunningTimerState(TimeEntryView *te);
    void displayStoppedTimerState();
    void displayLogin(
        const bool open,
        const uint64_t user_id);

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

private:
    Ui::MainWindowController *ui;

    bool shutdown;

    TogglApi *togglApi;

    bool tracking;
    bool loggedIn;

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

    void readSettings();
    void writeSettings();

    void connectMenuActions();
    void enableMenuActions();
};

#endif // MAINWINDOWCONTROLLER_H
