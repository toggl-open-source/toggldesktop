// Copyright 2014 Toggl Desktop developers.

#include "./errorviewcontroller.h"
#include "./ui_errorviewcontroller.h"
#include "./toggl.h"
#include "./bugsnag.h"

ErrorViewController::ErrorViewController(QWidget *parent)
    : QWidget(parent)
, ui(new Ui::ErrorViewController)
, networkError(false)
, loginError(false)
, uid(0) {
    ui->setupUi(this);
    setVisible(false);

    connect(TogglApi::instance, SIGNAL(displayError(QString,bool)),  // NOLINT
            this, SLOT(displayError(QString,bool)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayOnlineState(bool,QString)),  // NOLINT
            this, SLOT(displayOnlineState(bool,QString)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT
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
    loginError = !uid;
    ui->errorMessage->setText(errmsg);
    setVisible(true);
    if (!user_error) {
        Bugsnag::notify("error in shared lib", errmsg, "ErrorViewController");
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

void ErrorViewController::displayLogin(
    const bool open,
    const uint64_t user_id) {
    uid = user_id;
    Bugsnag::user.id = QString::number(user_id);
    if (user_id && isVisible() && loginError) {
        loginError = false;
        setVisible(false);
    }
}
