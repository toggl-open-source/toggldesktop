// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_FEEDBACKDIALOG_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_FEEDBACKDIALOG_H_

#include <QDialog>

namespace Ui {
class FeedbackDialog;
}

class FeedbackDialog : public QDialog {
    Q_OBJECT

 public:
    explicit FeedbackDialog(QWidget *parent = 0);
    ~FeedbackDialog();

 private slots:  // NOLINT
    void on_uploadImageButton_clicked();

    void on_sendButton_clicked();

 private:
    Ui::FeedbackDialog *ui;

    QString path;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_FEEDBACKDIALOG_H_
