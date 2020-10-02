// Copyright 2014 Toggl Desktop developers.

#include "loginwidget.h"
#include "ui_loginwidget.h"

#include "toggl.h"

#include <QKeyEvent>
#include <QtNetworkAuth>
#include <QDesktopServices>

#define OAUTH_SCOPE "email profile https://www.googleapis.com/auth/userinfo.profile https://www.googleapis.com/auth/userinfo.email openid"
#define OAUTH_AUTHORIZATION_URL "https://accounts.google.com/o/oauth2/v2/auth"
#define OAUTH_TOKEN_URL "https://accounts.google.com/o/oauth2/token"
#define OAUTH_CLIENT_ID "426090949585-uj7lka2mtanjgd7j9i6c4ik091rcv6n5.apps.googleusercontent.com"
#define OAUTH_CLIENT_KEY "6IHWKIfTAMF7cPJsBvoGxYui"

LoginWidget::LoginWidget(QStackedWidget *parent) : QWidget(parent),
ui(new Ui::LoginWidget) {
    ui->setupUi(this);

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(setCountries(QVector<CountryView * >)),  // NOLINT
            this, SLOT(setCountries(QVector<CountryView * >)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayError(QString,bool)),  // NOLINT
            this, SLOT(displayError(QString,bool)));  // NOLINT

    auto handler = new QOAuthHttpServerReplyHandler(this);
    handler->setCallbackText("Received verification code. You may now close this window.");

    oauth2.setReplyHandler(handler);
    oauth2.setAuthorizationUrl(QUrl(OAUTH_AUTHORIZATION_URL));
    oauth2.setAccessTokenUrl(QUrl(OAUTH_TOKEN_URL));
    oauth2.setClientIdentifier(OAUTH_CLIENT_ID);
    oauth2.setClientIdentifierSharedKey(OAUTH_CLIENT_KEY);
    oauth2.setScope(OAUTH_SCOPE);

    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::error, this, &LoginWidget::oauthError);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::granted, this, &LoginWidget::oauthGranted);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizationCallbackReceived, [=](const QVariantMap &data) {
        // The code gets somehow sometimes HTTP-escaped so fix that
        temporaryOAuthCode = data.value("code").toString();
        temporaryOAuthCode.replace("%2F", "/", Qt::CaseInsensitive);
        temporaryOAuthCode.replace("%3A", ":", Qt::CaseInsensitive);
        temporaryOAuthCode.replace("+", " ", Qt::CaseInsensitive);
    });
    oauth2.setModifyParametersFunction([this](QAbstractOAuth::Stage stage, QVariantMap* parameters) {
        if (stage == QAbstractOAuth::Stage::RequestingAccessToken) {
            parameters->insert("code", temporaryOAuthCode);
        }
    });

    connect(&oauth2, &QAbstractOAuth::authorizeWithBrowser, [=](QUrl url) {
        QUrlQuery query(url);
        url.setQuery(query);
        QMetaObject::invokeMethod(DesktopServices::instance(), "openUrl", Q_ARG(QUrl, url));
    });

    signupVisible = true;
    countriesLoaded = false;
    selectedCountryId = UINT64_MAX;

    on_viewchangelabel_linkActivated("");
}

LoginWidget::~LoginWidget() {
    delete ui;
}

void LoginWidget::displayError(
    const QString errmsg,
    const bool user_error) {
    Q_UNUSED(errmsg);
    Q_UNUSED(user_error);
    enableAllControls(true);
}

void LoginWidget::enableAllControls(const bool enable) {
    ui->email->setEnabled(enable);
    ui->password->setEnabled(enable);
    ui->login->setEnabled(enable);
    ui->signup->setEnabled(enable);
    ui->googleLogin->setEnabled(enable);
    ui->forgotPassword->setEnabled(enable);
    ui->viewchangelabel->setEnabled(enable);
}

void LoginWidget::display() {
    signupVisible = true;
    on_viewchangelabel_linkActivated("");
    qobject_cast<QStackedWidget*>(parent())->setCurrentWidget(this);
}

void LoginWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        if (signupVisible) {
            on_signup_clicked();
        } else {
            on_login_clicked();
        }
    }
}

void LoginWidget::mousePressEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    setFocus();
}

void LoginWidget::displayLogin(
    const bool open,
    const uint64_t user_id) {

    if (open) {
        display();
        ui->email->setFocus();
    }
    if (user_id) {
        ui->password->clear();
    }

    // Enable all
    enableAllControls(true);
}

void LoginWidget::on_login_clicked() {
    if (!validateFields(false)) {
        return;
    }
    enableAllControls(false);
    TogglApi::instance->login(ui->email->text(), ui->password->text());
}

void LoginWidget::on_googleLogin_linkActivated(const QString &link) {
    Q_UNUSED(link)
    oauth2.grant();
}

void LoginWidget::on_googleSignup_linkActivated(const QString &link) {
    Q_UNUSED(link)
    if (validateFields(true, true)) {
        oauth2.grant();
    }
}

void LoginWidget::oauthError(const QString &error, const QString &description, const QUrl &uri) {
    Q_UNUSED(uri);
    TogglApi::instance->displayError("Google error: " + error + " (" + description + ")", true);
}

void LoginWidget::oauthGranted() {
    if (signupVisible) {
        TogglApi::instance->googleSignup(oauth2.token(), selectedCountryId);
    }
    else {
        TogglApi::instance->googleLogin(oauth2.token());
    }
}

bool LoginWidget::validateFields(bool signup, bool google) {
    if (google)
        signup = true;
    if (!google) {
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
    }
    if (signup) {
        if (selectedCountryId == UINT64_MAX) {
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
    TogglApi::instance->signup(ui->email->text(), ui->password->text(), selectedCountryId);
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
    Q_UNUSED(link)
    if (signupVisible) {
        ui->signupWidget->hide();
        ui->loginWidget->show();
        ui->viewchangelabel->setText("<html><head/><body><a href='#' style='cursor:pointer;font-weight:bold;text-decoration:none;color:#fff;'>Sign up for free</a></body></html>");
        signupVisible = false;
    } else {
        ui->loginWidget->hide();
        ui->signupWidget->show();
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
        selectedCountryId = UINT64_MAX;
        return;
    }
    QVariant data = ui->countryComboBox->currentData();
    if (data.canConvert<CountryView *>()) {
        CountryView *view = data.value<CountryView *>();
        selectedCountryId = view->ID;
    }
}
