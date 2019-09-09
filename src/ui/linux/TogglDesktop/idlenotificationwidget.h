// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_IDLENOTIFICATIONWIDGET_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_IDLENOTIFICATIONWIDGET_H_

#include <QDialog>
#include <QTimer>
#include <QStackedWidget>
#include <QtDBus/QtDBus>

#include <stdint.h>

#include "./settingsview.h"

namespace Ui {
class IdleNotificationWidget;
}

class IdleNotificationWidget : public QWidget {
    Q_OBJECT

 public:
    explicit IdleNotificationWidget(QStackedWidget *parent = nullptr);
    ~IdleNotificationWidget();

    void display();
    void hide();

    bool isScreenLocked() const;

private:
    void storeIdlePeriod(uint64_t period);

 private slots:  // NOLINT
    void requestIdleHint();
    void idleHintReceived(QDBusPendingCallWatcher *watcher);

    void onScreensaverActiveChanged(bool active);

    void displaySettings(const bool open, SettingsView *settings);

    void displayStoppedTimerState();

    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void displayIdleNotification(
        const QString guid,
        const QString since,
        const QString duration,
        const uint64_t started,
        const QString description);

    void on_keepTimeButton_clicked();

    void on_discardTimeButton_clicked();

    void on_discardTimeAndContinueButton_clicked();

    void on_addIdleTimeAsNewEntryButton_clicked();

 private:
    Ui::IdleNotificationWidget *ui;
    QWidget *previousView;

    uint64_t idleStarted;
    QDBusInterface *screensaver;
    bool dbusApiAvailable;
    bool screenLocked;
    uint64_t lastActiveTime;

    QString timeEntryGUID;

    QTimer *idleHintTimer;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_IDLENOTIFICATIONWIDGET_H_
