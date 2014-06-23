#include "idlenotificationdialog.h"
#include "ui_idlenotificationdialog.h"

#include "toggl_api.h"

IdleNotificationDialog::IdleNotificationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IdleNotificationDialog),
    idleStarted(0)
{
    ui->setupUi(this);

    connect(TogglApi::instance, SIGNAL(displayIdleNotification(QString,QString,uint64_t)),
            this, SLOT(displayIdleNotification(QString,QString,uint64_t)));
}

IdleNotificationDialog::~IdleNotificationDialog()
{
    delete ui;
}

void IdleNotificationDialog::on_keepTimeButton_clicked()
{
    hide();
}

void IdleNotificationDialog::on_discardTimeButton_clicked()
{

}

void IdleNotificationDialog::displayIdleNotification(
    const QString since,
    const QString duration,
    const uint64_t started)
{
    idleStarted = started;

    ui->idleSince->setText(since);
    ui->idleDuration->setText(duration);

    show();
}
