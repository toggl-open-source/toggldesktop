// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_ERRORVIEWCONTROLLER_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_ERRORVIEWCONTROLLER_H_

#include <QWidget>

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

 private:
    Ui::ErrorViewController *ui;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_ERRORVIEWCONTROLLER_H_
