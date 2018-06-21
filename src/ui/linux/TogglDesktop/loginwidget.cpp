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

    connect(TogglApi::instance, SIGNAL(displayOverlay(int64_t)),  // NOLINT
            this, SLOT(displayOverlay(int64_t)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(setCountries(QVector<CountryView * >)),  // NOLINT
            this, SLOT(setCountries(QVector<CountryView * >)));  // NOLINT

    oauth2->setScope("profile email");
    oauth2->setAppName("Toggl Desktop");
    oauth2->setClientID("426090949585.apps.googleusercontent.com");
    oauth2->setRedirectURI("http://www.google.com/robots.txt");

    connect(oauth2, SIGNAL(loginDone()), this, SLOT(loginDone()));

    signupVisible = true;
    countriesLoaded = false;
    selectedCountryId = -1;

    on_viewchangelabel_linkActivated("");;
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

void LoginWidget::displayOverlay(const int64_t type) {
    setVisible(false);
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
    if (!validateFields(false)) {
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

bool LoginWidget::validateFields(const bool signup) {
    if (ui->email->text().isEmpty()) {
        ui->email->setFocus();
        TogglApi::instance->displayError(QString("Please enter valid email address"), true);
        return false;
    }
    if (ui->password->text().isEmpty()) {
        ui->password->setFocus();
        TogglApi::instance->displayError(QString("A password is required"), true);
        return false;
    }
    if (signup) {
        if (selectedCountryId == -1) {
            ui->countryComboBox->setFocus();
            TogglApi::instance->displayError(QString("Please select Country before signing up"), true);
            return false;
        }
        if (ui->tosCheckBox->checkState() == Qt::Unchecked) {
            ui->tosCheckBox->setFocus();
            TogglApi::instance->displayError(QString("You must agree to the terms of service and privacy policy to use Toggl"), true);
            return false;
        }
    }
    return true;
}

void LoginWidget::on_signup_clicked() {
    if (!validateFields(true)) {
        return;
    }
    TogglApi::instance->signup(ui->email->text(), ui->password->text());
}

void LoginWidget::setCountries(
    QVector<CountryView * > list) {
    ui->countryComboBox->clear();
    ui->countryComboBox->addItem("  -- Select country --   ");
    foreach(CountryView *view, list) {
        ui->countryComboBox->addItem(view->Text, QVariant::fromValue(view));
    }
}

void LoginWidget::on_viewchangelabel_linkActivated(const QString &link)
{
    if (signupVisible) {
        ui->signupFrame->hide();
        ui->loginFrame->show();
        ui->viewchangelabel->setText("<html><head/><body><a href='#' style='cursor:pointer;font-weight:bold;text-decoration:none;color:#fff;'>Sign up for free</a></body></html>");
        signupVisible = false;
    } else {
        ui->loginFrame->hide();
        ui->signupFrame->show();
        ui->viewchangelabel->setText("<html><head/><body><a href='#' style='cursor:pointer;font-weight:bold;text-decoration:none;color:#fff;'>Back to login</a></body></html>");
        signupVisible = true;
        if (!countriesLoaded) {
            TogglApi::instance->getCountries();
            countriesLoaded = true;
        }
    }
}

void LoginWidget::on_countryComboBox_currentIndexChanged(int index)
{
    if (index == 0) {
        selectedCountryId = -1;
        return;
    }
    QVariant data = ui->countryComboBox->currentData();
    if (data.canConvert<CountryView *>()) {
        CountryView *view = data.value<CountryView *>();
        selectedCountryId = view->ID;
    }
}
