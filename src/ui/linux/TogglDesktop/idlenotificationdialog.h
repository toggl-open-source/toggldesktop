#ifndef IDLENOTIFICATIONDIALOG_H
#define IDLENOTIFICATIONDIALOG_H

#include <QDialog>

#include <stdint.h>

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
    void displayIdleNotification(
        const QString since,
        const QString duration,
        const uint64_t started);

    void on_keepTimeButton_clicked();

    void on_discardTimeButton_clicked();

private:
    Ui::IdleNotificationDialog *ui;

    uint64_t idleStarted;
};

#endif // IDLENOTIFICATIONDIALOG_H
