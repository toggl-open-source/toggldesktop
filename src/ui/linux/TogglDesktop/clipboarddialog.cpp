#include "clipboarddialog.h"
#include "ui_clipboarddialog.h"

#include <QUrl>
#include <QClipboard>

ClipboardDialog::ClipboardDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClipboardDialog)
{
    ui->setupUi(this);
}

ClipboardDialog::~ClipboardDialog()
{
    delete ui;
}

void ClipboardDialog::showUrl(const QUrl &url) {
    ui->url->setText(url.toString());
    show();
    ui->url->setFocus();
    ui->url->selectAll();
}

void ClipboardDialog::on_copyButton_clicked() {
    auto clipboard = QGuiApplication::clipboard();
    clipboard->setText(ui->url->text());
}
