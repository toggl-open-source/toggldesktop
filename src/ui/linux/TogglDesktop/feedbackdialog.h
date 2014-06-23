#ifndef FEEDBACKDIALOG_H
#define FEEDBACKDIALOG_H

#include <QDialog>

namespace Ui {
class FeedbackDialog;
}

class FeedbackDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FeedbackDialog(QWidget *parent = 0);
    ~FeedbackDialog();

private slots:
    void on_uploadImageButton_clicked();

    void on_sendButton_clicked();

private:
    Ui::FeedbackDialog *ui;

    QString path;
};

#endif // FEEDBACKDIALOG_H
