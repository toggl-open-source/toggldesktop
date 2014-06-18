#ifndef MAINWINDOWCONTROLLER_H
#define MAINWINDOWCONTROLLER_H

#include <QMainWindow>
#include <QStackedWidget>

#include <stdbool.h>
#include <stdint.h>

#include "toggl_api.h"

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

public slots:
    void displayApp(const _Bool open);

    void displayError(
        const QString errmsg,
        const bool user_error);

    void displayUpdate(
        const bool open,
        UpdateView *update);

    void displayOnlineState(
        const bool is_online);

    void displayUrl(
        const QString url);

    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void displayReminder(
        const QString title,
        const QString informative_text);

    void displayTimeEntryList(
        const bool open,
        QVector<TimeEntryView *> list);

    void displayTimeEntryEditor(
        const bool open,
        TimeEntryView *te,
        const QString used_field_name);

    void displaySettings(
        const bool open,
        SettingsView *settings);

    void displayStoppedTimerState();

    void displayRunningTimerState(
        TimeEntryView *te);

    void displayIdleNotification(
        const QString since,
        const QString duration,
        const uint64_t started);

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindowController *ui;
    void *ctx_;
    bool shutdown_;
    TogglApi *togglApi;
    QStackedWidget *stackedWidget;

    void readSettings();
    void writeSettings();
};

#endif // MAINWINDOWCONTROLLER_H
