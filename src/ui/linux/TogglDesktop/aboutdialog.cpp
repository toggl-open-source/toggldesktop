// Copyright 2014 Toggl Desktop developers.

#include "./aboutdialog.h"
#include "./ui_aboutdialog.h"

#include <QDebug>  // NOLINT
#include <QDesktopServices>  // NOLINT
#include <QApplication>  // NOLINT

#include "./toggl.h"

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent),
ui(new Ui::AboutDialog) {
    ui->setupUi(this);

    ui->releaseChannel->addItem("stable");
    ui->releaseChannel->addItem("beta");
    ui->releaseChannel->addItem("dev");

    ui->version->setText(QApplication::applicationVersion());

    connect(TogglApi::instance, SIGNAL(displayUpdate(QString)),  // NOLINT
            this, SLOT(displayUpdate(QString)));  // NOLINT
}

AboutDialog::~AboutDialog() {
    delete ui;
}

void AboutDialog::displayUpdate(const QString update_url) {
    qDebug() << "displayUpdate update_url=" << update_url;

    url = update_url;

    QString channel = TogglApi::instance->updateChannel();
    ui->releaseChannel->setCurrentText(channel);
    ui->releaseChannel->setEnabled(true);
    ui->updateButton->setEnabled(!url.isEmpty());

    if (!url.isEmpty()) {
        ui->updateButton->setText(
            "Click here to download update!");
    } else {
        ui->updateButton->setText("Toggl Desktop is up to date");
    }
}

void AboutDialog::on_updateButton_clicked() {
    qDebug() << "on_updateButton_clicked url=" << url;
    QDesktopServices::openUrl(QUrl(url));
    TogglApi::instance->shutdown = true;
    qApp->exit(0);
}

void AboutDialog::on_releaseChannel_activated(const QString &arg1) {
    qDebug() << "on_releaseChannel_activated channel=" << arg1;
    ui->updateButton->setEnabled(false);
    ui->updateButton->setText("Checking for update");
    TogglApi::instance->setUpdateChannel(arg1);
}
