#include "loginwidget.h"
#include "ui_loginwidget.h"

#include "toggl_api.h"

LoginWidget::LoginWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWidget)
{
    ui->setupUi(this);

    setVisible(false);

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)), this, SLOT(displayLogin(bool,uint64_t)));
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::displayLogin(
    const bool open,
    const uint64_t user_id) {

    if (open) {
        setVisible(true);
    }
    if (user_id) {
        setVisible(false);
    }
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
