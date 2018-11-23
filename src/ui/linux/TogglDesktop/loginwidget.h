// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_LOGINWIDGET_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_LOGINWIDGET_H_

#include <QWidget>
#include <QVector>
#include <QStackedWidget>

#include <stdint.h>

#include "./timeentryview.h"
#include "./countryview.h"
#include "./oauth2.h"

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

    void displayOverlay(const int64_t type);

    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void displayTimeEntryList(
        const bool open,
        QVector<TimeEntryView *> list,
        const bool show_load_more_button);

    void on_googleLogin_linkActivated(const QString &link);

    void loginDone();
    bool validateFields(
        const bool signup);

    void on_signup_clicked();

    void setCountries(
        QVector<CountryView * > list);

    void on_viewchangelabel_linkActivated(const QString &link);

    void on_countryComboBox_currentIndexChanged(int index);

 private:
    Ui::LoginWidget *ui;

    OAuth2 *oauth2;

    bool signupVisible;

    bool countriesLoaded;
    int64_t selectedCountryId;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_LOGINWIDGET_H_
