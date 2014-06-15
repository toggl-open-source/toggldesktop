#include "loginviewcontroller.h"
#include "ui_loginviewcontroller.h"

LoginViewController::LoginViewController(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginViewController)
{
    ui->setupUi(this);
}

LoginViewController::~LoginViewController()
{
    delete ui;
}
