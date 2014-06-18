#include "loginwidget.h"
#include "ui_loginwidget.h"

#include "toggl_api.h"

LoginWidget::LoginWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWidget)
{
    ui->setupUi(this);
    setVisible(false);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::on_login_clicked()
{
    if (ui->email->text().isEmpty()) {
        ui->email->setFocus();
        return;
    }
    if (ui->password->text().isEmpty()) {
        ui->password->setFocus();
        return;
    }
    TogglApi::instance->login(ui->email->text(), ui->password->text());
}
