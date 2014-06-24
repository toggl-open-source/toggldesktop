#ifndef IDLENOTIFICATIONDIALOG_H
#define IDLENOTIFICATIONDIALOG_H

#include <QDialog>
#include <QTimer>

#include <stdint.h>

#include "settingsview.h"

namespace Ui {
class IdleNotificationDialog;
}

class IdleNotificationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IdleNotificationDialog(QWidget *parent = 0);
    ~IdleNotificationDialog();

private slots:
    void displayStoppedTimerState();

    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void displayIdleNotification(
        const QString since,
        const QString duration,
        const uint64_t started);

    void displaySettings(
        const bool open,
        SettingsView *settings);

    void on_keepTimeButton_clicked();

    void on_discardTimeButton_clicked();

    void timeout();

private:
    Ui::IdleNotificationDialog *ui;

    uint64_t idleStarted;

    QTimer *timer;
};

#endif // IDLENOTIFICATIONDIALOG_H
