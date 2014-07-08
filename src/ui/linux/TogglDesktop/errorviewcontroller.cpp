// Copyright 2014 Toggl Desktop developers.

#include "./errorviewcontroller.h"
#include "./ui_errorviewcontroller.h"
#include "./toggl.h"

ErrorViewController::ErrorViewController(QWidget *parent)
    : QWidget(parent)
, ui(new Ui::ErrorViewController)
, networkError(false) {
    ui->setupUi(this);
    setVisible(false);

    connect(TogglApi::instance, SIGNAL(displayError(QString,bool)),  // NOLINT
            this, SLOT(displayError(QString,bool)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayOnlineState(bool,QString)),  // NOLINT
            this, SLOT(displayOnlineState(bool,QString)));  // NOLINT
}

ErrorViewController::~ErrorViewController() {
    delete ui;
}

void ErrorViewController::on_pushButton_clicked() {
    setVisible(false);
}

void ErrorViewController::displayError(
    const QString errmsg,
    const bool user_error) {
    networkError = false;
    ui->errorMessage->setText(errmsg);
    setVisible(true);
    if (!user_error) {
        // FIXME: notify bugsnag
    }
}

void ErrorViewController::displayOnlineState(
    const bool is_online,
    const QString reason) {
    if (!is_online && !reason.isEmpty()) {
        networkError = true;
        ui->errorMessage->setText(reason);
        setVisible(true);
    } else if (is_online && networkError) {
        setVisible(false);
    }
}
