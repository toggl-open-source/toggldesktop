#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

#include "updateview.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = 0);
    ~AboutDialog();

private slots:
    void displayUpdate(const bool open, UpdateView *view);

    void on_updateButton_clicked();

    void on_releaseChannel_activated(const QString &arg1);

private:
    Ui::AboutDialog *ui;

    QString url;
};

#endif // ABOUTDIALOG_H
