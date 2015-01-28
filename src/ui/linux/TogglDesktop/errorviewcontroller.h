// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_ERRORVIEWCONTROLLER_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_ERRORVIEWCONTROLLER_H_

#include <QWidget>

#include <stdint.h>

namespace Ui {
class ErrorViewController;
}

class ErrorViewController : public QWidget {
    Q_OBJECT

 public:
    explicit ErrorViewController(QWidget *parent = 0);
    ~ErrorViewController();

 private slots:  // NOLINT
    void on_pushButton_clicked();

    void displayError(
        const QString errmsg,
        const bool user_error);

    void displayOnlineState(
        int64_t state);

    void displayLogin(
        const bool open,
        const uint64_t user_id);

 private:
    Ui::ErrorViewController *ui;

    bool networkError;
    bool loginError;
    uint64_t uid;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_ERRORVIEWCONTROLLER_H_
