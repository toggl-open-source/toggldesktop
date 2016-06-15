// Copyright 2014 Toggl Desktop developers.

#include <QKeyEvent>

#include "./loginwidget.h"
#include "./ui_loginwidget.h"

#include "./toggl.h"

LoginWidget::LoginWidget(QWidget *parent) : QWidget(parent),
ui(new Ui::LoginWidget),
oauth2(new OAuth2(this)) {
    ui->setupUi(this);

    setVisible(false);

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayTimeEntryList(bool,QVector<TimeEntryView*>,bool)),  // NOLINT
            this, SLOT(displayTimeEntryList(bool,QVector<TimeEntryView*>,bool)));  // NOLINT

    oauth2->setScope("profile email");
    oauth2->setAppName("Toggl Desktop");
    oauth2->setClientID("426090949585.apps.googleusercontent.com");
    oauth2->setRedirectURI("http://www.google.com/robots.txt");

    connect(oauth2, SIGNAL(loginDone()), this, SLOT(loginDone()));
}

LoginWidget::~LoginWidget() {
    delete ui;
}

void LoginWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        on_login_clicked();
    }
}

void LoginWidget::mousePressEvent(QMouseEvent* event) {
    setFocus();
}

void LoginWidget::displayLogin(
    const bool open,
    const uint64_t user_id) {

    if (open) {
        setVisible(true);
        ui->email->setFocus();
    }
    if (user_id) {
        setVisible(false);
    }
}

void LoginWidget::displayTimeEntryList(
    const bool open,
    QVector<TimeEntryView *> list,
    const bool) {
    if (open) {
        setVisible(false);
    }
}

void LoginWidget::on_login_clicked() {
    if (!validateFields()) {
        return;
    }
    TogglApi::instance->login(ui->email->text(), ui->password->text());
}

void LoginWidget::on_googleLogin_linkActivated(const QString &link) {
    oauth2->startLogin(true);
}

void LoginWidget::loginDone() {
    TogglApi::instance->googleLogin(oauth2->accessToken());
}

bool LoginWidget::validateFields() {
    if (ui->email->text().isEmpty()) {
        ui->email->setFocus();
        return false;
    }
    if (ui->password->text().isEmpty()) {
        ui->password->setFocus();
        return false;
    }
    return true;
}

void LoginWidget::on_signup_clicked() {
    if (!validateFields()) {
        return;
    }
    TogglApi::instance->signup(ui->email->text(), ui->password->text());
}
