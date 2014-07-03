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

 private slots:  // NOLINT
    void on_login_clicked();

    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void displayTimeEntryList(
        const bool open,
        QVector<TimeEntryView *> list);

    void on_googleLogin_linkActivated(const QString &link);

    void loginDone();

 private:
    Ui::LoginWidget *ui;

    OAuth2 *oauth2;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_LOGINWIDGET_H_
