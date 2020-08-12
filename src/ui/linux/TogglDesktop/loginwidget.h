// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_LOGINWIDGET_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_LOGINWIDGET_H_

#include <QWidget>
#include <QVector>
#include <QStackedWidget>
#include <QOAuth2AuthorizationCodeFlow>

#include <stdint.h>

#include "./timeentryview.h"
#include "./countryview.h"

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget {
    Q_OBJECT

 public:
    explicit LoginWidget(QStackedWidget *parent = nullptr);
    ~LoginWidget();

    void display();

 protected:
    virtual void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);

 private slots:  // NOLINT
    void on_login_clicked();

    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void on_googleLogin_linkActivated(const QString &link);
    void on_googleSignup_linkActivated(const QString &link);

    void oauthError(const QString &error, const QString &description, const QUrl &uri);
    void oauthGranted();
    bool validateFields(bool signup, bool google = false);

    void on_signup_clicked();

    void setCountries(
        QVector<CountryView * > list);

    void on_viewchangelabel_linkActivated(const QString &link);

    void on_countryComboBox_currentIndexChanged(int index);

    void displayError(
        const QString errmsg,
        const bool user_error);

 private:
    Ui::LoginWidget *ui;

    QOAuth2AuthorizationCodeFlow oauth2;
    QString temporaryOAuthCode;

    bool signupVisible;

    bool countriesLoaded;
    uint64_t selectedCountryId;

    void enableAllControls(const bool enable);
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_LOGINWIDGET_H_
