#ifndef CLIPBOARDDIALOG_H
#define CLIPBOARDDIALOG_H

#include <QDialog>

namespace Ui {
class ClipboardDialog;
}

class ClipboardDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ClipboardDialog(QWidget *parent = nullptr);
    ~ClipboardDialog();

    void showUrl(const QUrl &url);

private slots:
    void on_copyButton_clicked();


private:
    Ui::ClipboardDialog *ui;
};

#endif // CLIPBOARDDIALOG_H
