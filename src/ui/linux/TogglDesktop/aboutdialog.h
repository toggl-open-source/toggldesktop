// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_ABOUTDIALOG_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_ABOUTDIALOG_H_

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog {
    Q_OBJECT

 public:
    explicit AboutDialog(QWidget *parent = 0);
    ~AboutDialog();

 private slots:  // NOLINT
    void displayUpdate(const QString url);

    void on_updateButton_clicked();

    void on_releaseChannel_activated(const QString &arg1);

 private:
    Ui::AboutDialog *ui;

    QString url;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_ABOUTDIALOG_H_
