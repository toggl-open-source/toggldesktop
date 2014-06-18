#include "errorviewcontroller.h"
#include "ui_errorviewcontroller.h"

ErrorViewController::ErrorViewController(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ErrorViewController)
{
    ui->setupUi(this);
}

ErrorViewController::~ErrorViewController()
{
    delete ui;
}

void ErrorViewController::on_pushButton_clicked()
{
    setVisible(false);
}
