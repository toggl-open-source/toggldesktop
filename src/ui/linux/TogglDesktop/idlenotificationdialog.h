// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_IDLENOTIFICATIONDIALOG_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_IDLENOTIFICATIONDIALOG_H_

#include <QDialog>
#include <QTimer>

#include <stdint.h>

#include "./settingsview.h"

namespace Ui {
class IdleNotificationDialog;
}

class IdleNotificationDialog : public QDialog {
    Q_OBJECT

 public:
    explicit IdleNotificationDialog(QWidget *parent = 0);
    ~IdleNotificationDialog();

 private slots:  // NOLINT
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

    void displaySettings(
        const bool open,
        SettingsView *settings);

    void on_keepTimeButton_clicked();

    void on_discardTimeButton_clicked();

    void on_discardTimeAndContinueButton_clicked();

    void timeout();

    void on_pushButton_clicked();

 private:
    Ui::IdleNotificationDialog *ui;

    uint64_t idleStarted;

    QTimer *timer;

    QString timeEntryGUID;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_IDLENOTIFICATIONDIALOG_H_
