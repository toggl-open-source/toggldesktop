// Copyright 2014 Toggl Desktop developers.

#include "./errorviewcontroller.h"
#include "./ui_errorviewcontroller.h"
#include "./toggl.h"

ErrorViewController::ErrorViewController(QWidget *parent)
    : QWidget(parent)
, ui(new Ui::ErrorViewController)
, loginError(false)
, uid(0) {
    ui->setupUi(this);
    setVisible(false);

    connect(TogglApi::instance, SIGNAL(displayError(QString,bool)),  // NOLINT
            this, SLOT(displayError(QString,bool)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayOnlineState(int64_t)),  // NOLINT
            this, SLOT(displayOnlineState(int64_t)));  // NOLINT

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
    loginError = !uid;
    ui->errorMessage->setText(errmsg);
    ui->errorMessage->setToolTip(
        QString("<p style='color:white;background-color:black;'>" +
                errmsg + "</p>"));
    setVisible(true);
    if (!user_error) {
        TogglApi::notifyBugsnag("error in shared lib", errmsg,
                                "ErrorViewController");
    }
}

void ErrorViewController::displayOnlineState(
    int64_t state) {
    // FIXME: need separate online state label
}

void ErrorViewController::displayLogin(
    const bool open,
    const uint64_t user_id) {
    uid = user_id;
    if (user_id && isVisible() && loginError) {
        loginError = false;
        setVisible(false);
    }
}
