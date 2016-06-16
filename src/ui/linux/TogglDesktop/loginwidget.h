// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_LOGINWIDGET_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_LOGINWIDGET_H_

#include <QWidget>
#include <QVector>

#include <stdint.h>

#include "./timeentryview.h"
#include "./oauth2.h"

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget {
    Q_OBJECT

 public:
    explicit LoginWidget(QWidget *parent = 0);
    ~LoginWidget();

 protected:
    virtual void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);

 private slots:  // NOLINT
    void on_login_clicked();

    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void displayTimeEntryList(
        const bool open,
        QVector<TimeEntryView *> list,
        const bool show_load_more_button);

    void on_googleLogin_linkActivated(const QString &link);

    void loginDone();
    bool validateFields();

    void on_signup_clicked();

 private:
    Ui::LoginWidget *ui;

    OAuth2 *oauth2;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_LOGINWIDGET_H_
